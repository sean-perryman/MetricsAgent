#pragma once
#include <string>

struct MetricsToggles {
  bool cpu = true;
  bool memory = true;
  bool disk = true;
  bool volumes = true;
  bool network = true;
  bool users = true;
};

struct JsonExporterConfig {
  bool enabled = true;
  std::wstring endpoint;
  std::wstring apiKey;
};

struct PromExporterConfig {
  bool enabled = false;
  std::wstring pushgateway;
  std::wstring job = L"metricsagent";
};

struct OtlpExporterConfig {
  bool enabled = false;
  std::wstring endpoint;
};

struct AppConfig {
  int intervalSeconds = 30;
  int timeoutSeconds = 10;
  std::wstring hostnameOverride;

  MetricsToggles metrics;

  JsonExporterConfig json;
  PromExporterConfig prom;
  OtlpExporterConfig otlp;
};

bool loadConfigFromFile(const std::wstring& path, AppConfig& out, std::wstring& err);
