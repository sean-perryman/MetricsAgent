#pragma once
#include "agent.h"
#include "config.h"

class OtlpExporter : public IExporter {
public:
  explicit OtlpExporter(const OtlpExporterConfig& cfg, int timeoutSeconds);
  void exportMetrics(const MetricsSnapshot& snap) override;

private:
  OtlpExporterConfig cfg_;
  int timeoutSeconds_;
};
