#define UNICODE
#include "exporters/exporter_prometheus.h"
#include "http_client.h"
#include <string>
#include <sstream>
#include <codecvt>
#include <locale>

static std::string wsToUtf8(const std::wstring& ws) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
  return conv.to_bytes(ws);
}

PrometheusExporter::PrometheusExporter(const PromExporterConfig& cfg, int timeoutSeconds)
  : cfg_(cfg), timeoutSeconds_(timeoutSeconds) {}

static void addLine(std::ostringstream& o, const std::string& name, const std::string& labels, double v) {
  o << name << labels << " " << v << "\n";
}
static void addLineU64(std::ostringstream& o, const std::string& name, const std::string& labels, unsigned long long v) {
  o << name << labels << " " << v << "\n";
}

void PrometheusExporter::exportMetrics(const MetricsSnapshot& snap) {
  if (!cfg_.enabled) return;

  std::string host = wsToUtf8(snap.hostname);
  std::ostringstream o;

  // Prometheus text format (Pushgateway accepts this)
  o << "# TYPE metricsagent_cpu_utilization_pct gauge\n";
  addLine(o, "metricsagent_cpu_utilization_pct", "{host=\"" + host + "\"}", snap.cpuPct);
  o << "# TYPE metricsagent_cpu_idle_pct gauge\n";
  addLine(o, "metricsagent_cpu_idle_pct", "{host=\"" + host + "\"}", snap.idlePct);

  o << "# TYPE metricsagent_memory_total_mb gauge\n";
  addLineU64(o, "metricsagent_memory_total_mb", "{host=\"" + host + "\"}", snap.memory.totalMB);
  o << "# TYPE metricsagent_memory_used_mb gauge\n";
  addLineU64(o, "metricsagent_memory_used_mb", "{host=\"" + host + "\"}", snap.memory.usedMB);
  o << "# TYPE metricsagent_memory_free_mb gauge\n";
  addLineU64(o, "metricsagent_memory_free_mb", "{host=\"" + host + "\"}", snap.memory.freeMB);

  o << "# TYPE metricsagent_disk_reads_per_sec gauge\n";
  o << "# TYPE metricsagent_disk_writes_per_sec gauge\n";
  o << "# TYPE metricsagent_disk_queue_length gauge\n";
  o << "# TYPE metricsagent_disk_read_latency_ms gauge\n";
  o << "# TYPE metricsagent_disk_write_latency_ms gauge\n";
  o << "# TYPE metricsagent_disk_utilization_pct gauge\n";
  o << "# TYPE metricsagent_disk_reads_total counter\n";
  o << "# TYPE metricsagent_disk_writes_total counter\n";

  for (const auto& d : snap.disks) {
    std::string disk = wsToUtf8(d.disk);
    std::string labels = "{host=\"" + host + "\",disk=\"" + disk + "\"}";
    addLine(o, "metricsagent_disk_reads_per_sec", labels, d.readsPerSec);
    addLine(o, "metricsagent_disk_writes_per_sec", labels, d.writesPerSec);
    addLine(o, "metricsagent_disk_queue_length", labels, d.queueLength);
    addLine(o, "metricsagent_disk_read_latency_ms", labels, d.readLatencyMs);
    addLine(o, "metricsagent_disk_write_latency_ms", labels, d.writeLatencyMs);
    addLine(o, "metricsagent_disk_utilization_pct", labels, d.utilizationPct);
    addLine(o, "metricsagent_disk_reads_total", labels, d.readsTotal);
    addLine(o, "metricsagent_disk_writes_total", labels, d.writesTotal);
  }

  o << "# TYPE metricsagent_volume_total_bytes gauge\n";
  o << "# TYPE metricsagent_volume_free_bytes gauge\n";
  o << "# TYPE metricsagent_volume_free_pct gauge\n";
  for (const auto& v : snap.volumes) {
    std::string root = wsToUtf8(v.root);
    std::string labels = "{host=\"" + host + "\",volume=\"" + root + "\"}";
    addLineU64(o, "metricsagent_volume_total_bytes", labels, v.totalBytes);
    addLineU64(o, "metricsagent_volume_free_bytes", labels, v.freeBytes);
    addLine(o, "metricsagent_volume_free_pct", labels, v.freePct);
  }

  o << "# TYPE metricsagent_net_bytes_per_sec gauge\n";
  o << "# TYPE metricsagent_net_bytes_total counter\n";
  for (const auto& n : snap.network) {
    std::string iface = wsToUtf8(n.iface);
    std::string labels = "{host=\"" + host + "\",iface=\"" + iface + "\"}";
    addLine(o, "metricsagent_net_bytes_per_sec", labels, n.bytesPerSec);
    addLine(o, "metricsagent_net_bytes_total", labels, n.bytesTotal);
  }

  // Pushgateway endpoint: /metrics/job/<job>/instance/<host>
  std::wstring url = cfg_.pushgateway;
  if (!url.empty() && url.back() == L'/') url.pop_back();
  url += L"/metrics/job/" + cfg_.job + L"/instance/" + snap.hostname;

  std::wstring headers = L"Content-Type: text/plain; version=0.0.4\r\n";
  httpPostUtf8(url, headers, o.str(), timeoutSeconds_);
}
