#define UNICODE
#include "exporters/exporter_otlp.h"
#include "http_client.h"
#include "json_util.h"
#include <string>
#include <sstream>
#include <codecvt>
#include <locale>

static std::string wsToUtf8(const std::wstring& ws) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
  return conv.to_bytes(ws);
}

OtlpExporter::OtlpExporter(const OtlpExporterConfig& cfg, int timeoutSeconds)
  : cfg_(cfg), timeoutSeconds_(timeoutSeconds) {}

static void otlpGauge(std::ostringstream& o, const std::string& name, double val) {
  o << "{"
    << "\"name\":\"" << jsonEscapeUtf8(name) << "\","
    << "\"gauge\":{\"dataPoints\":[{\"asDouble\":" << val << "}]}"
    << "}";
}

static void otlpSum(std::ostringstream& o, const std::string& name, double val, bool monotonic=true) {
  o << "{"
    << "\"name\":\"" << jsonEscapeUtf8(name) << "\","
    << "\"sum\":{\"isMonotonic\":" << (monotonic ? "true":"false")
    << ",\"aggregationTemporality\":2," // CUMULATIVE
    << "\"dataPoints\":[{\"asDouble\":" << val << "}]}"
    << "}";
}

void OtlpExporter::exportMetrics(const MetricsSnapshot& snap) {
  if (!cfg_.enabled) return;

  std::string host = wsToUtf8(snap.hostname);
  std::string mid  = wsToUtf8(snap.machineId);

  std::ostringstream metrics;
  bool first = true;
  auto add = [&](const std::string& obj){
    if (!first) metrics << ",";
    metrics << obj;
    first = false;
  };

  { std::ostringstream t; otlpGauge(t, "system.cpu.utilization", snap.cpuPct / 100.0); add(t.str()); }
  { std::ostringstream t; otlpGauge(t, "system.cpu.idle", snap.idlePct / 100.0); add(t.str()); }
  { std::ostringstream t; otlpGauge(t, "system.memory.used.mb", (double)snap.memory.usedMB); add(t.str()); }
  { std::ostringstream t; otlpGauge(t, "system.memory.total.mb", (double)snap.memory.totalMB); add(t.str()); }

  for (const auto& d : snap.disks) {
    // In a full OTEL SDK we'd attach attributes per datapoint. This simplified exporter emits per-disk metric names.
    { std::ostringstream t; otlpGauge(t, "system.disk.reads_per_sec." + wsToUtf8(d.disk), d.readsPerSec); add(t.str()); }
    { std::ostringstream t; otlpGauge(t, "system.disk.writes_per_sec." + wsToUtf8(d.disk), d.writesPerSec); add(t.str()); }
    { std::ostringstream t; otlpGauge(t, "system.disk.queue_length." + wsToUtf8(d.disk), d.queueLength); add(t.str()); }
    { std::ostringstream t; otlpGauge(t, "system.disk.read_latency_ms." + wsToUtf8(d.disk), d.readLatencyMs); add(t.str()); }
    { std::ostringstream t; otlpGauge(t, "system.disk.write_latency_ms." + wsToUtf8(d.disk), d.writeLatencyMs); add(t.str()); }
    { std::ostringstream t; otlpGauge(t, "system.disk.utilization_pct." + wsToUtf8(d.disk), d.utilizationPct); add(t.str()); }
    { std::ostringstream t; otlpSum(t, "system.disk.reads_total." + wsToUtf8(d.disk), d.readsTotal, true); add(t.str()); }
    { std::ostringstream t; otlpSum(t, "system.disk.writes_total." + wsToUtf8(d.disk), d.writesTotal, true); add(t.str()); }
  }

  for (const auto& v : snap.volumes) {
    { std::ostringstream t; otlpGauge(t, "system.volume.free_pct." + wsToUtf8(v.root), v.freePct/100.0); add(t.str()); }
  }

  for (const auto& n : snap.network) {
    { std::ostringstream t; otlpGauge(t, "system.net.bytes_per_sec." + wsToUtf8(n.iface), n.bytesPerSec); add(t.str()); }
    { std::ostringstream t; otlpSum(t, "system.net.bytes_total." + wsToUtf8(n.iface), n.bytesTotal, true); add(t.str()); }
  }

  // Minimal OTLP/HTTP JSON (collector should accept JSON encoding; many do via otlphttpjson)
  std::ostringstream body;
  body << "{"
       << "\"resourceMetrics\":[{"
       << "\"resource\":{\"attributes\":["
       << "{\"key\":\"host.name\",\"value\":{\"stringValue\":\"" << jsonEscapeUtf8(host) << "\"}},"
       << "{\"key\":\"host.id\",\"value\":{\"stringValue\":\"" << jsonEscapeUtf8(mid) << "\"}}"
       << "]},\"scopeMetrics\":[{"
       << "\"scope\":{\"name\":\"metricsagent\",\"version\":\"1.0\"},"
       << "\"metrics\":[" << metrics.str() << "]"
       << "}]}]}";
  body << "}";

  std::wstring headers = L"Content-Type: application/json\r\n";
  httpPostUtf8(cfg_.endpoint, headers, body.str(), timeoutSeconds_);
}
