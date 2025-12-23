#define UNICODE
#include "agent.h"
#include "identifiers.h"
#include "config.h"
#include "metrics.h"
#include "exporters/exporter_json.h"
#include "exporters/exporter_prometheus.h"
#include "exporters/exporter_otlp.h"

#include <windows.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

// forward declarations for metric functions
double collectCPU();
MemoryMetrics collectMemory();
std::vector<DiskMetrics> collectDisk();
std::vector<VolumeMetrics> collectVolumes();
std::vector<NetworkMetrics> collectNetwork();
std::vector<std::wstring> collectUsers();

class CPUCollector : public ICollector {
public: void collect(MetricsSnapshot& out) override {
  out.cpuPct = collectCPU();
  out.idlePct = 100.0 - out.cpuPct;
}};

class MemoryCollector : public ICollector {
public: void collect(MetricsSnapshot& out) override {
  out.memory = collectMemory();
}};

class DiskCollector : public ICollector {
public: void collect(MetricsSnapshot& out) override {
  out.disks = collectDisk();
}};

class VolumeCollector : public ICollector {
public: void collect(MetricsSnapshot& out) override {
  out.volumes = collectVolumes();
}};

class NetworkCollector : public ICollector {
public: void collect(MetricsSnapshot& out) override {
  out.network = collectNetwork();
}};

class UsersCollector : public ICollector {
public: void collect(MetricsSnapshot& out) override {
  out.users = collectUsers();
}};

std::vector<std::unique_ptr<ICollector>> buildCollectors(const AppConfig& cfg) {
  std::vector<std::unique_ptr<ICollector>> collectors;
  if (cfg.metrics.cpu) collectors.push_back(std::make_unique<CPUCollector>());
  if (cfg.metrics.memory) collectors.push_back(std::make_unique<MemoryCollector>());
  if (cfg.metrics.disk) collectors.push_back(std::make_unique<DiskCollector>());
  if (cfg.metrics.volumes) collectors.push_back(std::make_unique<VolumeCollector>());
  if (cfg.metrics.network) collectors.push_back(std::make_unique<NetworkCollector>());
  if (cfg.metrics.users) collectors.push_back(std::make_unique<UsersCollector>());
  return collectors;
}

std::vector<std::unique_ptr<IExporter>> buildExporters(const AppConfig& cfg) {
  std::vector<std::unique_ptr<IExporter>> exps;
  if (cfg.json.enabled) exps.push_back(std::make_unique<JsonExporter>(cfg.json, cfg.timeoutSeconds));
  if (cfg.prom.enabled) exps.push_back(std::make_unique<PrometheusExporter>(cfg.prom, cfg.timeoutSeconds));
  if (cfg.otlp.enabled) exps.push_back(std::make_unique<OtlpExporter>(cfg.otlp, cfg.timeoutSeconds));
  return exps;
}

void runAgentLoop(const AppConfig& cfg) {
  auto collectors = buildCollectors(cfg);
  auto exporters = buildExporters(cfg);

  MetricsSnapshot snap{};
  snap.hostname = getHostname(cfg.hostnameOverride);
  snap.machineId = getMachineGuid();

  while (true) {
    snap.timestamp = std::chrono::system_clock::now();

    // collect
    for (auto& c : collectors) c->collect(snap);

    // export
    for (auto& e : exporters) e->exportMetrics(snap);

    std::this_thread::sleep_for(std::chrono::seconds(cfg.intervalSeconds));
  }
}
