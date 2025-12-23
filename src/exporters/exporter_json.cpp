#define UNICODE
#include "exporters/exporter_json.h"
#include "http_client.h"
#include "json_util.h"
#include <sstream>
#include <codecvt>
#include <locale>

static std::string wsToUtf8(const std::wstring& ws){ std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> c; return c.to_bytes(ws); }
JsonExporter::JsonExporter(const JsonExporterConfig& cfg,int timeoutSeconds):cfg_(cfg),timeoutSeconds_(timeoutSeconds){}
void JsonExporter::exportMetrics(const MetricsSnapshot& s){
  if(!cfg_.enabled) return;
  std::ostringstream o;
  o<<"{"
   <<"\"hostname\":\""<<jsonEscapeUtf8(wsToUtf8(s.hostname))<<"\","
   <<"\"machine_id\":\""<<jsonEscapeUtf8(wsToUtf8(s.machineId))<<"\","
   <<"\"timestamp_unix_ms\":"<<(long long)std::chrono::duration_cast<std::chrono::milliseconds>(s.timestamp.time_since_epoch()).count()<<","
   <<"\"cpu\":{\"utilization_pct\":"<<s.cpuPct<<",\"idle_pct\":"<<s.idlePct<<"},"
   <<"\"memory\":{\"total_mb\":"<<s.memory.totalMB<<",\"used_mb\":"<<s.memory.usedMB<<",\"free_mb\":"<<s.memory.freeMB<<"},";
  o<<"\"disks\":[";
  for(size_t i=0;i<s.disks.size();i++){ const auto& d=s.disks[i];
    o<<"{\"disk\":\""<<jsonEscapeUtf8(wsToUtf8(d.disk))<<"\","
     <<"\"reads_per_sec\":"<<d.readsPerSec<<",\"writes_per_sec\":"<<d.writesPerSec<<",\"queue_length\":"<<d.queueLength<<","
     <<"\"read_latency_ms\":"<<d.readLatencyMs<<",\"write_latency_ms\":"<<d.writeLatencyMs<<",\"utilization_pct\":"<<d.utilizationPct<<","
     <<"\"reads_total\":"<<d.readsTotal<<",\"writes_total\":"<<d.writesTotal<<"}";
    if(i+1<s.disks.size()) o<<",";
  }
  o<<"],\"volumes\":[";
  for(size_t i=0;i<s.volumes.size();i++){ const auto& v=s.volumes[i];
    o<<"{\"root\":\""<<jsonEscapeUtf8(wsToUtf8(v.root))<<"\","
     <<"\"total_bytes\":"<<v.totalBytes<<",\"free_bytes\":"<<v.freeBytes<<",\"free_pct\":"<<v.freePct<<"}";
    if(i+1<s.volumes.size()) o<<",";
  }
  o<<"],\"network\":[";
  for(size_t i=0;i<s.network.size();i++){ const auto& n=s.network[i];
    o<<"{\"iface\":\""<<jsonEscapeUtf8(wsToUtf8(n.iface))<<"\","
     <<"\"bytes_per_sec\":"<<n.bytesPerSec<<",\"bytes_total\":"<<n.bytesTotal<<"}";
    if(i+1<s.network.size()) o<<",";
  }
  o<<"],\"users\":[";
  for(size_t i=0;i<s.users.size();i++){ o<<"\""<<jsonEscapeUtf8(wsToUtf8(s.users[i]))<<"\""; if(i+1<s.users.size()) o<<","; }
  o<<"]}";
  std::wstring headers=L"Content-Type: application/json\r\n";
  if(!cfg_.apiKey.empty()) headers += L"Authorization: Bearer " + cfg_.apiKey + L"\r\n";
  httpPostUtf8(cfg_.endpoint, headers, o.str(), timeoutSeconds_);
}
