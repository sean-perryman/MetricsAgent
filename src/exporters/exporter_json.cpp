#define UNICODE
#include "exporters/exporter_json.h"
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

JsonExporter::JsonExporter(const JsonExporterConfig& cfg, int timeoutSeconds)
  : cfg_(cfg), timeoutSeconds_(timeoutSeconds) {}

void JsonExporter::exportMetrics(const MetricsSnapshot& snap) {
  if (!cfg_.enabled) return;

  // Build UTF-8 JSON payload
  std::ostringstream o;
  auto host = wsToUtf8(snap.hostname);
  auto mid  = wsToUtf8(snap.machineId);

  o << "{";
  o << "\"hostname\":\"" << jsonEscapeUtf8(host) << "\",";
  o << "\"machine_id\":\"" << jsonEscapeUtf8(mid) << "\",";
  o << "\"timestamp_unix_ms\":"
    << (long long)std::chrono::duration_cast<std::chrono::milliseconds>(snap.timestamp.time_since_epoch()).count() << ",";

  o << "\"cpu\":{"
    << "\"utilization_pct\":" << snap.cpuPct << ","
    << "\"idle_pct\":" << snap.idlePct
    << "},";

  o << "\"memory\":{"
    << "\"total_mb\":" << snap.memory.totalMB << ","
    << "\"used_mb\":"  << snap.memory.usedMB  << ","
    << "\"free_mb\":"  << snap.memory.freeMB
    << "},";

  o << "\"disks\":[";
  for (size_t i=0;i<snap.disks.size();i++) {
    const auto& d = snap.disks[i];
    o << "{";
    o << "\"disk\":\"" << jsonEscapeUtf8(wsToUtf8(d.disk)) << "\",";
    o << "\"reads_per_sec\":" << d.readsPerSec << ",";
    o << "\"writes_per_sec\":" << d.writesPerSec << ",";
    o << "\"queue_length\":" << d.queueLength << ",";
    o << "\"read_latency_ms\":" << d.readLatencyMs << ",";
    o << "\"write_latency_ms\":" << d.writeLatencyMs << ",";
    o << "\"utilization_pct\":" << d.utilizationPct << ",";
    o << "\"reads_total\":" << d.readsTotal << ",";
    o << "\"writes_total\":" << d.writesTotal;
    o << "}";
    if (i+1<snap.disks.size()) o << ",";
  }
  o << "],";

  o << "\"volumes\":[";
  for (size_t i=0;i<snap.volumes.size();i++) {
    const auto& v = snap.volumes[i];
    o << "{";
    o << "\"root\":\"" << jsonEscapeUtf8(wsToUtf8(v.root)) << "\",";
    o << "\"total_bytes\":" << v.totalBytes << ",";
    o << "\"free_bytes\":" << v.freeBytes << ",";
    o << "\"free_pct\":" << v.freePct;
    o << "}";
    if (i+1<snap.volumes.size()) o << ",";
  }
  o << "],";

  o << "\"network\":[";
  for (size_t i=0;i<snap.network.size();i++) {
    const auto& n = snap.network[i];
    o << "{";
    o << "\"iface\":\"" << jsonEscapeUtf8(wsToUtf8(n.iface)) << "\",";
    o << "\"bytes_per_sec\":" << n.bytesPerSec << ",";
    o << "\"bytes_total\":" << n.bytesTotal;
    o << "}";
    if (i+1<snap.network.size()) o << ",";
  }
  o << "],";

  o << "\"users\":[";
  for (size_t i=0;i<snap.users.size();i++) {
    o << "\"" << jsonEscapeUtf8(wsToUtf8(snap.users[i])) << "\"";
    if (i+1<snap.users.size()) o << ",";
  }
  o << "]";

  o << "}";

  std::wstring headers = L"Content-Type: application/json\r\n";
  if (!cfg_.apiKey.empty()) headers += L"Authorization: Bearer " + cfg_.apiKey + L"\r\n";

  httpPostUtf8(cfg_.endpoint, headers, o.str(), timeoutSeconds_);
}
