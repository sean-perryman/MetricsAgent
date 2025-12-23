#pragma once
#include "agent.h"
#include "config.h"

class JsonExporter : public IExporter {
public:
  explicit JsonExporter(const JsonExporterConfig& cfg, int timeoutSeconds);
  void exportMetrics(const MetricsSnapshot& snap) override;

private:
  JsonExporterConfig cfg_;
  int timeoutSeconds_;
};
