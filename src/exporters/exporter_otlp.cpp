#define UNICODE
#include "exporters/exporter_otlp.h"
#include "http_client.h"
#include "json_util.h"
#include <sstream>
#include <codecvt>
#include <locale>

static std::string wsToUtf8(const std::wstring& ws){ std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> c; return c.to_bytes(ws); }
OtlpExporter::OtlpExporter(const OtlpExporterConfig& cfg,int timeoutSeconds):cfg_(cfg),timeoutSeconds_(timeoutSeconds){}

static std::string gauge(const std::string& name,double v){
  std::ostringstream o; o<<"{\"name\":\""<<jsonEscapeUtf8(name)<<"\",\"gauge\":{\"dataPoints\":[{\"asDouble\":"<<v<<"}]}}"; return o.str();
}
static std::string sum(const std::string& name,double v){
  std::ostringstream o; o<<"{\"name\":\""<<jsonEscapeUtf8(name)<<"\",\"sum\":{\"isMonotonic\":true,\"aggregationTemporality\":2,\"dataPoints\":[{\"asDouble\":"<<v<<"}]}}"; return o.str();
}

void OtlpExporter::exportMetrics(const MetricsSnapshot& s){
  if(!cfg_.enabled) return;
  std::string host=wsToUtf8(s.hostname), mid=wsToUtf8(s.machineId);

  std::ostringstream m; bool first=true;
  auto add=[&](const std::string& obj){ if(!first) m<<","; m<<obj; first=false; };

  add(gauge("system.cpu.utilization", s.cpuPct/100.0));
  add(gauge("system.cpu.idle", s.idlePct/100.0));
  add(gauge("system.memory.used.mb", (double)s.memory.usedMB));
  add(gauge("system.memory.total.mb", (double)s.memory.totalMB));

  for(const auto& d: s.disks){
    std::string suf=wsToUtf8(d.disk);
    add(gauge("system.disk.reads_per_sec."+suf, d.readsPerSec));
    add(gauge("system.disk.writes_per_sec."+suf, d.writesPerSec));
    add(gauge("system.disk.queue_length."+suf, d.queueLength));
    add(gauge("system.disk.read_latency_ms."+suf, d.readLatencyMs));
    add(gauge("system.disk.write_latency_ms."+suf, d.writeLatencyMs));
    add(gauge("system.disk.utilization_pct."+suf, d.utilizationPct));
    add(sum("system.disk.reads_total."+suf, d.readsTotal));
    add(sum("system.disk.writes_total."+suf, d.writesTotal));
  }
  for(const auto& n: s.network){
    std::string suf=wsToUtf8(n.iface);
    add(gauge("system.net.bytes_per_sec."+suf, n.bytesPerSec));
    add(sum("system.net.bytes_total."+suf, n.bytesTotal));
  }

  std::ostringstream body;
  body<<"{\"resourceMetrics\":[{\"resource\":{\"attributes\":[{\"key\":\"host.name\",\"value\":{\"stringValue\":\""
      <<jsonEscapeUtf8(host)<<"\"}},{\"key\":\"host.id\",\"value\":{\"stringValue\":\""<<jsonEscapeUtf8(mid)
      <<"\"}}]},\"scopeMetrics\":[{\"scope\":{\"name\":\"metricsagent\",\"version\":\"2.0\"},\"metrics\":["
      <<m.str()<<"]}]}]}";
  std::wstring headers=L"Content-Type: application/json\r\n";
  httpPostUtf8(cfg_.endpoint, headers, body.str(), timeoutSeconds_);
}
