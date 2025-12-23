#define UNICODE
#include "config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

static bool findBool(const std::wstring& s, const std::wstring& key, bool def) {
  auto pos = s.find(key);
  if (pos == std::wstring::npos) return def;
  auto t = s.substr(pos);
  auto truePos = t.find(L"true");
  auto falsePos = t.find(L"false");
  if (truePos != std::wstring::npos && (falsePos == std::wstring::npos || truePos < falsePos)) return true;
  if (falsePos != std::wstring::npos) return false;
  return def;
}

static int findInt(const std::wstring& s, const std::wstring& key, int def) {
  auto pos = s.find(key);
  if (pos == std::wstring::npos) return def;
  pos = s.find(L":", pos);
  if (pos == std::wstring::npos) return def;
  pos++;
  while (pos < s.size() && (s[pos] == L' ' || s[pos] == L'\t')) pos++;
  std::wstring num;
  while (pos < s.size() && (s[pos] >= L'0' && s[pos] <= L'9')) num.push_back(s[pos++]);
  if (num.empty()) return def;
  return std::stoi(num);
}

static std::wstring findString(const std::wstring& s, const std::wstring& key, const std::wstring& def) {
  auto pos = s.find(key);
  if (pos == std::wstring::npos) return def;
  pos = s.find(L":", pos);
  if (pos == std::wstring::npos) return def;
  pos = s.find(L"\"", pos);
  if (pos == std::wstring::npos) return def;
  pos++;
  auto end = s.find(L"\"", pos);
  if (end == std::wstring::npos) return def;
  return s.substr(pos, end - pos);
}

bool loadConfigFromFile(const std::wstring& path, AppConfig& out, std::wstring& err) {
  std::wifstream f(path);
  if (!f) {
    err = L"Failed to open config: " + path;
    return false;
  }
  std::wstringstream ss;
  ss << f.rdbuf();
  std::wstring s = ss.str();

  out.intervalSeconds = findInt(s, L"\"interval_seconds\"", out.intervalSeconds);
  out.timeoutSeconds = findInt(s, L"\"timeout_seconds\"", out.timeoutSeconds);
  out.hostnameOverride = findString(s, L"\"hostname_override\"", out.hostnameOverride);

  out.metrics.cpu = findBool(s, L"\"cpu\"", out.metrics.cpu);
  out.metrics.memory = findBool(s, L"\"memory\"", out.metrics.memory);
  out.metrics.disk = findBool(s, L"\"disk\"", out.metrics.disk);
  out.metrics.volumes = findBool(s, L"\"volumes\"", out.metrics.volumes);
  out.metrics.network = findBool(s, L"\"network\"", out.metrics.network);
  out.metrics.users = findBool(s, L"\"users\"", out.metrics.users);

  out.json.enabled = findBool(s, L"\"json\"", out.json.enabled);
  out.json.endpoint = findString(s, L"\"endpoint\"", out.json.endpoint);
  out.json.apiKey = findString(s, L"\"api_key\"", out.json.apiKey);

  out.prom.enabled = findBool(s, L"\"prometheus\"", out.prom.enabled);
  out.prom.pushgateway = findString(s, L"\"pushgateway\"", out.prom.pushgateway);
  out.prom.job = findString(s, L"\"job\"", out.prom.job);

  out.otlp.enabled = findBool(s, L"\"otlp\"", out.otlp.enabled);
  out.otlp.endpoint = findString(s, L"\"endpoint\"", out.otlp.endpoint);

  return true;
}
