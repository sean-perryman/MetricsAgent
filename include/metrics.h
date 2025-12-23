#pragma once
#include <string>
#include <vector>
#include <chrono>

struct MemoryMetrics {
  unsigned long long totalMB = 0;
  unsigned long long usedMB = 0;
  unsigned long long freeMB = 0;
};

struct DiskMetrics {
  std::wstring disk;           // PhysicalDisk instance name
  double readsPerSec = 0.0;
  double writesPerSec = 0.0;
  double queueLength = 0.0;
  double readLatencyMs = 0.0;
  double writeLatencyMs = 0.0;
  double utilizationPct = 0.0;

  // integrated totals (approx monotonic)
  double readsTotal = 0.0;
  double writesTotal = 0.0;
};

struct VolumeMetrics {
  std::wstring root;           // e.g. C:\
  unsigned long long totalBytes = 0;
  unsigned long long freeBytes = 0;
  double freePct = 0.0;
};

struct NetworkMetrics {
  std::wstring iface;
  double bytesPerSec = 0.0;

  // integrated total (approx monotonic)
  double bytesTotal = 0.0;
};

struct MetricsSnapshot {
  std::wstring hostname;
  std::wstring machineId;
  std::chrono::system_clock::time_point timestamp;

  double cpuPct = 0.0;
  double idlePct = 0.0;
  MemoryMetrics memory;

  std::vector<DiskMetrics> disks;
  std::vector<VolumeMetrics> volumes;
  std::vector<NetworkMetrics> network;

  std::vector<std::wstring> users;
};

class ICollector {
public:
  virtual ~ICollector() = default;
  virtual void collect(MetricsSnapshot& out) = 0;
};
