#define UNICODE
#include <pdh.h>
#include "metrics.h"
#pragma comment(lib,"pdh.lib")
static PDH_HQUERY q=nullptr; static PDH_HCOUNTER c=nullptr; static bool init=false;
static void initCPU(){ if(init) return; PdhOpenQueryW(nullptr,0,&q); PdhAddEnglishCounterW(q,L"\\Processor(_Total)\\% Processor Time",0,&c); PdhCollectQueryData(q); init=true; }
double collectCPU(){ initCPU(); PdhCollectQueryData(q); PDH_FMT_COUNTERVALUE v{}; PdhGetFormattedCounterValue(c,PDH_FMT_DOUBLE,nullptr,&v); return v.doubleValue; }
