#pragma once
#include <memory>
#include <vector>
#include "config.h"
#include "metrics.h"
class IExporter{public: virtual ~IExporter()=default; virtual void exportMetrics(const MetricsSnapshot& snap)=0;};
std::vector<std::unique_ptr<ICollector>> buildCollectors(const AppConfig& cfg);
std::vector<std::unique_ptr<IExporter>> buildExporters(const AppConfig& cfg);
void runAgentLoop(const AppConfig& cfg);
