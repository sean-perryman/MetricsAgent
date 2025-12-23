#define UNICODE
#include <pdh.h>
#include "metrics.h"

#pragma comment(lib, "pdh.lib")

static PDH_HQUERY query = nullptr;
static PDH_HCOUNTER cpuCounter = nullptr;
static bool initialized = false;

static void initCPU() {
  if (initialized) return;
  PdhOpenQueryW(nullptr, 0, &query);
  PdhAddEnglishCounterW(query, L"\\Processor(_Total)\\% Processor Time", 0, &cpuCounter);
  PdhCollectQueryData(query); // warmup
  initialized = true;
}

double collectCPU() {
  initCPU();
  PdhCollectQueryData(query);
  PDH_FMT_COUNTERVALUE val{};
  PdhGetFormattedCounterValue(cpuCounter, PDH_FMT_DOUBLE, nullptr, &val);
  return val.doubleValue;
}
