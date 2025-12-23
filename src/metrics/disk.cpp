#define UNICODE
#include <windows.h>
#include <pdh.h>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include "metrics.h"

#pragma comment(lib, "pdh.lib")

struct DiskCounterSet {
  PDH_HCOUNTER reads = nullptr;
  PDH_HCOUNTER writes = nullptr;
  PDH_HCOUNTER queue = nullptr;
  PDH_HCOUNTER readLat = nullptr;
  PDH_HCOUNTER writeLat = nullptr;
  PDH_HCOUNTER util = nullptr;
};

static PDH_HQUERY diskQuery = nullptr;
static std::map<std::wstring, DiskCounterSet> diskCounters;
static std::map<std::wstring, DiskMetrics> diskState;
static bool diskInit = false;
static std::chrono::steady_clock::time_point lastSample;

static void initDiskCounters() {
  if (diskInit) return;
  PdhOpenQueryW(nullptr, 0, &diskQuery);

  // enumerate instances
  DWORD instSize = 0;
  PdhEnumObjectItemsW(nullptr, nullptr, L"PhysicalDisk",
    nullptr, nullptr, nullptr, &instSize, PERF_DETAIL_WIZARD, 0);

  std::vector<wchar_t> instances(instSize + 2);
  DWORD size2 = (DWORD)instances.size();
  PdhEnumObjectItemsW(nullptr, nullptr, L"PhysicalDisk",
    nullptr, nullptr, instances.data(), &size2, PERF_DETAIL_WIZARD, 0);

  for (wchar_t* p = instances.data(); *p; p += wcslen(p) + 1) {
    if (wcscmp(p, L"_Total") == 0) continue;
    DiskCounterSet set{};
    std::wstring base = L"\\PhysicalDisk(" + std::wstring(p) + L")\\";

    PdhAddEnglishCounterW(diskQuery, (base + L"Disk Reads/sec").c_str(), 0, &set.reads);
    PdhAddEnglishCounterW(diskQuery, (base + L"Disk Writes/sec").c_str(), 0, &set.writes);
    PdhAddEnglishCounterW(diskQuery, (base + L"Avg. Disk Queue Length").c_str(), 0, &set.queue);
    PdhAddEnglishCounterW(diskQuery, (base + L"Avg. Disk sec/Read").c_str(), 0, &set.readLat);
    PdhAddEnglishCounterW(diskQuery, (base + L"Avg. Disk sec/Write").c_str(), 0, &set.writeLat);
    PdhAddEnglishCounterW(diskQuery, (base + L"% Disk Time").c_str(), 0, &set.util);

    diskCounters[p] = set;
    DiskMetrics s{}; s.disk = p;
    diskState[p] = s;
  }

  PdhCollectQueryData(diskQuery); // warmup
  lastSample = std::chrono::steady_clock::now();
  diskInit = true;
}

std::vector<DiskMetrics> collectDisk() {
  initDiskCounters();
  PdhCollectQueryData(diskQuery);

  auto now = std::chrono::steady_clock::now();
  double dt = std::chrono::duration<double>(now - lastSample).count();
  if (dt <= 0) dt = 1.0;
  lastSample = now;

  std::vector<DiskMetrics> results;
  PDH_FMT_COUNTERVALUE v{};

  for (auto& kv : diskCounters) {
    const std::wstring& name = kv.first;
    auto& c = kv.second;
    auto& state = diskState[name];

    DiskMetrics d = state; // start from state to keep totals
    d.disk = name;

    PdhGetFormattedCounterValue(c.reads, PDH_FMT_DOUBLE, nullptr, &v);
    d.readsPerSec = v.doubleValue;

    PdhGetFormattedCounterValue(c.writes, PDH_FMT_DOUBLE, nullptr, &v);
    d.writesPerSec = v.doubleValue;

    PdhGetFormattedCounterValue(c.queue, PDH_FMT_DOUBLE, nullptr, &v);
    d.queueLength = v.doubleValue;

    PdhGetFormattedCounterValue(c.readLat, PDH_FMT_DOUBLE, nullptr, &v);
    d.readLatencyMs = v.doubleValue * 1000.0;

    PdhGetFormattedCounterValue(c.writeLat, PDH_FMT_DOUBLE, nullptr, &v);
    d.writeLatencyMs = v.doubleValue * 1000.0;

    PdhGetFormattedCounterValue(c.util, PDH_FMT_DOUBLE, nullptr, &v);
    d.utilizationPct = v.doubleValue;

    // Delta math: integrate rates into monotonic totals (approx)
    d.readsTotal  += d.readsPerSec * dt;
    d.writesTotal += d.writesPerSec * dt;

    // update state
    state = d;
    results.push_back(d);
  }

  return results;
}

std::vector<VolumeMetrics> collectVolumes() {
  std::vector<VolumeMetrics> vols;
  DWORD mask = GetLogicalDrives();
  for (wchar_t d = L'A'; d <= L'Z'; d++) {
    if (!(mask & (1 << (d - L'A')))) continue;
    wchar_t root[] = { d, L':', L'\\', 0 };
    ULARGE_INTEGER freeB{}, totalB{};
    if (GetDiskFreeSpaceExW(root, &freeB, &totalB, nullptr)) {
      VolumeMetrics vm{};
      vm.root = root;
      vm.totalBytes = totalB.QuadPart;
      vm.freeBytes = freeB.QuadPart;
      vm.freePct = (totalB.QuadPart > 0) ? (double)freeB.QuadPart * 100.0 / (double)totalB.QuadPart : 0.0;
      vols.push_back(vm);
    }
  }
  return vols;
}
