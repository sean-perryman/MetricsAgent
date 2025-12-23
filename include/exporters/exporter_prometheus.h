#pragma once
#include "agent.h"
#include "config.h"

class PrometheusExporter : public IExporter {
public:
  explicit PrometheusExporter(const PromExporterConfig& cfg, int timeoutSeconds);
  void exportMetrics(const MetricsSnapshot& snap) override;

private:
  PromExporterConfig cfg_;
  int timeoutSeconds_;
};
