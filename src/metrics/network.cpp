#define UNICODE
#include <pdh.h>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include "metrics.h"

#pragma comment(lib, "pdh.lib")

static PDH_HQUERY netQuery = nullptr;
static std::map<std::wstring, PDH_HCOUNTER> netCounters;
static std::map<std::wstring, NetworkMetrics> netState;
static bool netInit = false;
static std::chrono::steady_clock::time_point netLastSample;

static void initNetwork() {
  if (netInit) return;
  PdhOpenQueryW(nullptr, 0, &netQuery);

  DWORD instSize = 0;
  PdhEnumObjectItemsW(nullptr, nullptr, L"Network Interface",
    nullptr, nullptr, nullptr, &instSize, PERF_DETAIL_WIZARD, 0);
  std::vector<wchar_t> instances(instSize + 2);
  DWORD size2 = (DWORD)instances.size();
  PdhEnumObjectItemsW(nullptr, nullptr, L"Network Interface",
    nullptr, nullptr, instances.data(), &size2, PERF_DETAIL_WIZARD, 0);

  for (wchar_t* p = instances.data(); *p; p += wcslen(p) + 1) {
    PDH_HCOUNTER c = nullptr;
    std::wstring path = L"\\Network Interface(" + std::wstring(p) + L")\\Bytes Total/sec";
    if (PdhAddEnglishCounterW(netQuery, path.c_str(), 0, &c) == ERROR_SUCCESS) {
      netCounters[p] = c;
      NetworkMetrics nm{}; nm.iface = p;
      netState[p] = nm;
    }
  }

  PdhCollectQueryData(netQuery); // warmup
  netLastSample = std::chrono::steady_clock::now();
  netInit = true;
}

std::vector<NetworkMetrics> collectNetwork() {
  initNetwork();
  PdhCollectQueryData(netQuery);

  auto now = std::chrono::steady_clock::now();
  double dt = std::chrono::duration<double>(now - netLastSample).count();
  if (dt <= 0) dt = 1.0;
  netLastSample = now;

  std::vector<NetworkMetrics> out;
  PDH_FMT_COUNTERVALUE v{};

  for (auto& kv : netCounters) {
    const std::wstring& name = kv.first;
    PDH_HCOUNTER c = kv.second;
    auto& state = netState[name];

    NetworkMetrics m = state;
    m.iface = name;

    PdhGetFormattedCounterValue(c, PDH_FMT_DOUBLE, nullptr, &v);
    m.bytesPerSec = v.doubleValue;

    // Delta math: integrate rate into monotonic total (approx)
    m.bytesTotal += m.bytesPerSec * dt;

    state = m;
    out.push_back(m);
  }
  return out;
}
