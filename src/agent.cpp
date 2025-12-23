#define UNICODE
#include "agent.h"
#include "identifiers.h"
#include "exporters/exporter_json.h"
#include "exporters/exporter_prometheus.h"
#include "exporters/exporter_otlp.h"
#include <thread>
#include <chrono>

double collectCPU();
MemoryMetrics collectMemory();
std::vector<DiskMetrics> collectDisk();
std::vector<VolumeMetrics> collectVolumes();
std::vector<NetworkMetrics> collectNetwork();
std::vector<std::wstring> collectUsers();

class CPUCollector: public ICollector{ public: void collect(MetricsSnapshot& o) override { o.cpuPct=collectCPU(); o.idlePct=100.0-o.cpuPct; } };
class MemCollector: public ICollector{ public: void collect(MetricsSnapshot& o) override { o.memory=collectMemory(); } };
class DiskCollector: public ICollector{ public: void collect(MetricsSnapshot& o) override { o.disks=collectDisk(); } };
class VolCollector: public ICollector{ public: void collect(MetricsSnapshot& o) override { o.volumes=collectVolumes(); } };
class NetCollector: public ICollector{ public: void collect(MetricsSnapshot& o) override { o.network=collectNetwork(); } };
class UserCollector: public ICollector{ public: void collect(MetricsSnapshot& o) override { o.users=collectUsers(); } };

std::vector<std::unique_ptr<ICollector>> buildCollectors(const AppConfig& cfg){
  std::vector<std::unique_ptr<ICollector>> c;
  if(cfg.metrics.cpu) c.push_back(std::make_unique<CPUCollector>());
  if(cfg.metrics.memory) c.push_back(std::make_unique<MemCollector>());
  if(cfg.metrics.disk) c.push_back(std::make_unique<DiskCollector>());
  if(cfg.metrics.volumes) c.push_back(std::make_unique<VolCollector>());
  if(cfg.metrics.network) c.push_back(std::make_unique<NetCollector>());
  if(cfg.metrics.users) c.push_back(std::make_unique<UserCollector>());
  return c;
}

std::vector<std::unique_ptr<IExporter>> buildExporters(const AppConfig& cfg){
  std::vector<std::unique_ptr<IExporter>> e;
  if(cfg.json.enabled) e.push_back(std::make_unique<JsonExporter>(cfg.json, cfg.timeoutSeconds));
  if(cfg.prom.enabled) e.push_back(std::make_unique<PrometheusExporter>(cfg.prom, cfg.timeoutSeconds));
  if(cfg.otlp.enabled) e.push_back(std::make_unique<OtlpExporter>(cfg.otlp, cfg.timeoutSeconds));
  return e;
}

void runAgentLoop(const AppConfig& cfg){
  auto collectors = buildCollectors(cfg);
  auto exporters  = buildExporters(cfg);

  MetricsSnapshot snap{};
  snap.hostname = getHostname(cfg.hostnameOverride);
  snap.machineId = getMachineGuid();

  while(true){
    snap.timestamp = std::chrono::system_clock::now();
    for(auto& col: collectors) col->collect(snap);
    for(auto& exp: exporters) exp->exportMetrics(snap);
    std::this_thread::sleep_for(std::chrono::seconds(cfg.intervalSeconds));
  }
}
