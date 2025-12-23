#define UNICODE
#include <windows.h>
#include "metrics.h"
MemoryMetrics collectMemory(){ MEMORYSTATUSEX m{}; m.dwLength=sizeof(m); GlobalMemoryStatusEx(&m); MemoryMetrics out; out.totalMB=m.ullTotalPhys/(1024ULL*1024ULL); out.freeMB=m.ullAvailPhys/(1024ULL*1024ULL); out.usedMB=(m.ullTotalPhys-m.ullAvailPhys)/(1024ULL*1024ULL); return out; }
