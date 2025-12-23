#pragma once
#include <string>
#include <vector>
#include <chrono>

struct MemoryMetrics { unsigned long long totalMB=0, usedMB=0, freeMB=0; };

struct DiskMetrics {
  std::wstring disk;
  double readsPerSec=0, writesPerSec=0, queueLength=0, readLatencyMs=0, writeLatencyMs=0, utilizationPct=0;
  double readsTotal=0, writesTotal=0;
};

struct VolumeMetrics { std::wstring root; unsigned long long totalBytes=0, freeBytes=0; double freePct=0; };

struct NetworkMetrics { std::wstring iface; double bytesPerSec=0, bytesTotal=0; };

struct MetricsSnapshot {
  std::wstring hostname;
  std::wstring machineId;
  std::chrono::system_clock::time_point timestamp;
  double cpuPct=0, idlePct=0;
  MemoryMetrics memory;
  std::vector<DiskMetrics> disks;
  std::vector<VolumeMetrics> volumes;
  std::vector<NetworkMetrics> network;
  std::vector<std::wstring> users;
};

class ICollector { public: virtual ~ICollector()=default; virtual void collect(MetricsSnapshot& out)=0; };
