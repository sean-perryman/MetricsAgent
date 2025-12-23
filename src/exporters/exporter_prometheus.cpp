#define UNICODE
#include "exporters/exporter_prometheus.h"
#include "http_client.h"
#include <sstream>
#include <codecvt>
#include <locale>

static std::string wsToUtf8(const std::wstring& ws){ std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> c; return c.to_bytes(ws); }
PrometheusExporter::PrometheusExporter(const PromExporterConfig& cfg,int timeoutSeconds):cfg_(cfg),timeoutSeconds_(timeoutSeconds){}
static void line(std::ostringstream& o,const std::string& n,const std::string& l,double v){ o<<n<<l<<" "<<v<<"\n"; }
static void lineU(std::ostringstream& o,const std::string& n,const std::string& l,unsigned long long v){ o<<n<<l<<" "<<v<<"\n"; }

void PrometheusExporter::exportMetrics(const MetricsSnapshot& s){
  if(!cfg_.enabled) return;
  std::string host=wsToUtf8(s.hostname);
  std::ostringstream o;
  o<<"# TYPE metricsagent_cpu_utilization_pct gauge\n"; line(o,"metricsagent_cpu_utilization_pct","{host=\""+host+"\"}",s.cpuPct);
  o<<"# TYPE metricsagent_cpu_idle_pct gauge\n"; line(o,"metricsagent_cpu_idle_pct","{host=\""+host+"\"}",s.idlePct);
  o<<"# TYPE metricsagent_memory_total_mb gauge\n"; lineU(o,"metricsagent_memory_total_mb","{host=\""+host+"\"}",s.memory.totalMB);
  o<<"# TYPE metricsagent_memory_used_mb gauge\n"; lineU(o,"metricsagent_memory_used_mb","{host=\""+host+"\"}",s.memory.usedMB);
  o<<"# TYPE metricsagent_memory_free_mb gauge\n"; lineU(o,"metricsagent_memory_free_mb","{host=\""+host+"\"}",s.memory.freeMB);

  o<<"# TYPE metricsagent_disk_reads_per_sec gauge\n# TYPE metricsagent_disk_writes_per_sec gauge\n# TYPE metricsagent_disk_queue_length gauge\n";
  o<<"# TYPE metricsagent_disk_read_latency_ms gauge\n# TYPE metricsagent_disk_write_latency_ms gauge\n# TYPE metricsagent_disk_utilization_pct gauge\n";
  o<<"# TYPE metricsagent_disk_reads_total counter\n# TYPE metricsagent_disk_writes_total counter\n";
  for(const auto& d: s.disks){
    std::string disk=wsToUtf8(d.disk);
    std::string L="{host=\""+host+"\",disk=\""+disk+"\"}";
    line(o,"metricsagent_disk_reads_per_sec",L,d.readsPerSec);
    line(o,"metricsagent_disk_writes_per_sec",L,d.writesPerSec);
    line(o,"metricsagent_disk_queue_length",L,d.queueLength);
    line(o,"metricsagent_disk_read_latency_ms",L,d.readLatencyMs);
    line(o,"metricsagent_disk_write_latency_ms",L,d.writeLatencyMs);
    line(o,"metricsagent_disk_utilization_pct",L,d.utilizationPct);
    line(o,"metricsagent_disk_reads_total",L,d.readsTotal);
    line(o,"metricsagent_disk_writes_total",L,d.writesTotal);
  }

  o<<"# TYPE metricsagent_volume_total_bytes gauge\n# TYPE metricsagent_volume_free_bytes gauge\n# TYPE metricsagent_volume_free_pct gauge\n";
  for(const auto& v: s.volumes){
    std::string vol=wsToUtf8(v.root);
    std::string L="{host=\""+host+"\",volume=\""+vol+"\"}";
    lineU(o,"metricsagent_volume_total_bytes",L,v.totalBytes);
    lineU(o,"metricsagent_volume_free_bytes",L,v.freeBytes);
    line(o,"metricsagent_volume_free_pct",L,v.freePct);
  }

  o<<"# TYPE metricsagent_net_bytes_per_sec gauge\n# TYPE metricsagent_net_bytes_total counter\n";
  for(const auto& n: s.network){
    std::string iface=wsToUtf8(n.iface);
    std::string L="{host=\""+host+"\",iface=\""+iface+"\"}";
    line(o,"metricsagent_net_bytes_per_sec",L,n.bytesPerSec);
    line(o,"metricsagent_net_bytes_total",L,n.bytesTotal);
  }

  std::wstring url=cfg_.pushgateway; if(!url.empty() && url.back()==L'/') url.pop_back();
  url += L"/metrics/job/" + cfg_.job + L"/instance/" + s.hostname;
  std::wstring headers=L"Content-Type: text/plain; version=0.0.4\r\n";
  httpPostUtf8(url, headers, o.str(), timeoutSeconds_);
}
