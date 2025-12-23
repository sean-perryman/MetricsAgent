#define UNICODE
#include <pdh.h>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include "metrics.h"
#pragma comment(lib,"pdh.lib")

static PDH_HQUERY Q=nullptr;
static std::map<std::wstring,PDH_HCOUNTER> cs;
static std::map<std::wstring,NetworkMetrics> st;
static bool inited=false;
static std::chrono::steady_clock::time_point lastT;

static void initNet(){
  if(inited) return;
  PdhOpenQueryW(nullptr,0,&Q);
  DWORD instSize=0;
  PdhEnumObjectItemsW(nullptr,nullptr,L"Network Interface",nullptr,nullptr,nullptr,&instSize,PERF_DETAIL_WIZARD,0);
  std::vector<wchar_t> inst(instSize+2);
  DWORD size2=(DWORD)inst.size();
  PdhEnumObjectItemsW(nullptr,nullptr,L"Network Interface",nullptr,nullptr,inst.data(),&size2,PERF_DETAIL_WIZARD,0);
  for(wchar_t* p=inst.data(); *p; p+=wcslen(p)+1){
    PDH_HCOUNTER c=nullptr;
    std::wstring path=L"\\\\Network Interface(" + std::wstring(p) + L")\\\\Bytes Total/sec";
    if(PdhAddEnglishCounterW(Q,path.c_str(),0,&c)==ERROR_SUCCESS){ cs[p]=c; NetworkMetrics nm{}; nm.iface=p; st[p]=nm; }
  }
  PdhCollectQueryData(Q);
  lastT=std::chrono::steady_clock::now();
  inited=true;
}

std::vector<NetworkMetrics> collectNetwork(){
  initNet();
  PdhCollectQueryData(Q);
  auto now=std::chrono::steady_clock::now();
  double dt=std::chrono::duration<double>(now-lastT).count(); if(dt<=0) dt=1.0; lastT=now;
  std::vector<NetworkMetrics> out; PDH_FMT_COUNTERVALUE v{};
  for(auto& kv: cs){
    const auto& name=kv.first; auto& state=st[name];
    NetworkMetrics m=state; m.iface=name;
    PdhGetFormattedCounterValue(kv.second,PDH_FMT_DOUBLE,nullptr,&v); m.bytesPerSec=v.doubleValue;
    m.bytesTotal += m.bytesPerSec*dt;
    state=m; out.push_back(m);
  }
  return out;
}
