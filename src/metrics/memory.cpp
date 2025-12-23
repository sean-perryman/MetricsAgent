#define UNICODE
#include <windows.h>
#include "metrics.h"

MemoryMetrics collectMemory() {
  MEMORYSTATUSEX mem{};
  mem.dwLength = sizeof(mem);
  GlobalMemoryStatusEx(&mem);

  MemoryMetrics m;
  m.totalMB = mem.ullTotalPhys / (1024ULL * 1024ULL);
  m.freeMB  = mem.ullAvailPhys / (1024ULL * 1024ULL);
  m.usedMB  = (mem.ullTotalPhys - mem.ullAvailPhys) / (1024ULL * 1024ULL);
  return m;
}
