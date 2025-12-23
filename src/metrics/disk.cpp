#define UNICODE
#include <windows.h>
#include <pdh.h>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include "metrics.h"
#pragma comment(lib,"pdh.lib")

struct CSet{ PDH_HCOUNTER r=nullptr,w=nullptr,q=nullptr,rl=nullptr,wl=nullptr,u=nullptr; };
static PDH_HQUERY Q=nullptr;
static std::map<std::wstring,CSet> cs;
static std::map<std::wstring,DiskMetrics> st;
static bool inited=false;
static std::chrono::steady_clock::time_point lastT;

static void initDisk(){
  if(inited) return;
  PdhOpenQueryW(nullptr,0,&Q);
  DWORD instSize=0;
  PdhEnumObjectItemsW(nullptr,nullptr,L"PhysicalDisk",nullptr,nullptr,nullptr,&instSize,PERF_DETAIL_WIZARD,0);
  std::vector<wchar_t> inst(instSize+2);
  DWORD size2=(DWORD)inst.size();
  PdhEnumObjectItemsW(nullptr,nullptr,L"PhysicalDisk",nullptr,nullptr,inst.data(),&size2,PERF_DETAIL_WIZARD,0);
  for(wchar_t* p=inst.data(); *p; p+=wcslen(p)+1){
    if(wcscmp(p,L"_Total")==0) continue;
    std::wstring base=L"\\\\PhysicalDisk(" + std::wstring(p) + L")\\\\";
    CSet s{};
    PdhAddEnglishCounterW(Q,(base+L"Disk Reads/sec").c_str(),0,&s.r);
    PdhAddEnglishCounterW(Q,(base+L"Disk Writes/sec").c_str(),0,&s.w);
    PdhAddEnglishCounterW(Q,(base+L"Avg. Disk Queue Length").c_str(),0,&s.q);
    PdhAddEnglishCounterW(Q,(base+L"Avg. Disk sec/Read").c_str(),0,&s.rl);
    PdhAddEnglishCounterW(Q,(base+L"Avg. Disk sec/Write").c_str(),0,&s.wl);
    PdhAddEnglishCounterW(Q,(base+L"% Disk Time").c_str(),0,&s.u);
    cs[p]=s; DiskMetrics dm{}; dm.disk=p; st[p]=dm;
  }
  PdhCollectQueryData(Q);
  lastT=std::chrono::steady_clock::now();
  inited=true;
}

std::vector<DiskMetrics> collectDisk(){
  initDisk();
  PdhCollectQueryData(Q);
  auto now=std::chrono::steady_clock::now();
  double dt=std::chrono::duration<double>(now-lastT).count(); if(dt<=0) dt=1.0; lastT=now;
  std::vector<DiskMetrics> out; PDH_FMT_COUNTERVALUE v{};
  for(auto& kv: cs){
    const auto& name=kv.first; auto& c=kv.second; auto& state=st[name];
    DiskMetrics d=state; d.disk=name;
    PdhGetFormattedCounterValue(c.r,PDH_FMT_DOUBLE,nullptr,&v); d.readsPerSec=v.doubleValue;
    PdhGetFormattedCounterValue(c.w,PDH_FMT_DOUBLE,nullptr,&v); d.writesPerSec=v.doubleValue;
    PdhGetFormattedCounterValue(c.q,PDH_FMT_DOUBLE,nullptr,&v); d.queueLength=v.doubleValue;
    PdhGetFormattedCounterValue(c.rl,PDH_FMT_DOUBLE,nullptr,&v); d.readLatencyMs=v.doubleValue*1000.0;
    PdhGetFormattedCounterValue(c.wl,PDH_FMT_DOUBLE,nullptr,&v); d.writeLatencyMs=v.doubleValue*1000.0;
    PdhGetFormattedCounterValue(c.u,PDH_FMT_DOUBLE,nullptr,&v); d.utilizationPct=v.doubleValue;
    d.readsTotal += d.readsPerSec*dt;
    d.writesTotal += d.writesPerSec*dt;
    state=d; out.push_back(d);
  }
  return out;
}

std::vector<VolumeMetrics> collectVolumes(){
  std::vector<VolumeMetrics> vols;
  DWORD mask=GetLogicalDrives();
  for(wchar_t d=L'A'; d<=L'Z'; d++){
    if(!(mask & (1<<(d-L'A')))) continue;
    wchar_t root[]={d,L':',L'\\',0};
    ULARGE_INTEGER freeB{}, totalB{};
    if(GetDiskFreeSpaceExW(root,&freeB,&totalB,nullptr)){
      VolumeMetrics vm{}; vm.root=root; vm.totalBytes=totalB.QuadPart; vm.freeBytes=freeB.QuadPart;
      vm.freePct = (totalB.QuadPart>0) ? (double)freeB.QuadPart*100.0/(double)totalB.QuadPart : 0.0;
      vols.push_back(vm);
    }
  }
  return vols;
}
