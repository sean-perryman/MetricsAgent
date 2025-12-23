#define UNICODE
#include "config.h"
#include <windows.h>
#include <cstdio>
#include <string>

static std::wstring readFileWide(const std::wstring& path, std::wstring& err){
  FILE* f = _wfopen(path.c_str(), L"rb");
  if(!f){ err = L"Failed to open config: " + path; return L""; }
  std::string bytes; bytes.reserve(8192);
  char buf[4096]; size_t n=0;
  while((n=fread(buf,1,sizeof(buf),f))>0) bytes.append(buf, buf+n);
  fclose(f);

  auto toWide = [&](UINT cp)->std::wstring{
    int wlen = MultiByteToWideChar(cp, 0, bytes.data(), (int)bytes.size(), nullptr, 0);
    if(wlen<=0) return L"";
    std::wstring ws(wlen, L'\0');
    MultiByteToWideChar(cp, 0, bytes.data(), (int)bytes.size(), ws.data(), wlen);
    return ws;
  };
  std::wstring ws = toWide(CP_UTF8);
  if(ws.empty()) ws = toWide(CP_ACP);
  return ws;
}

static bool findBool(const std::wstring& s, const std::wstring& key, bool def){
  auto pos = s.find(key); if(pos==std::wstring::npos) return def;
  auto t = s.substr(pos);
  auto tp=t.find(L"true"), fp=t.find(L"false");
  if(tp!=std::wstring::npos && (fp==std::wstring::npos || tp<fp)) return true;
  if(fp!=std::wstring::npos) return false;
  return def;
}
static int findInt(const std::wstring& s, const std::wstring& key, int def){
  auto pos=s.find(key); if(pos==std::wstring::npos) return def;
  pos=s.find(L":",pos); if(pos==std::wstring::npos) return def; pos++;
  while(pos<s.size() && (s[pos]==L' '||s[pos]==L'\t')) pos++;
  std::wstring num;
  while(pos<s.size() && (s[pos]>=L'0'&&s[pos]<=L'9')) num.push_back(s[pos++]);
  if(num.empty()) return def;
  return std::stoi(num);
}
static std::wstring findString(const std::wstring& s, const std::wstring& key, const std::wstring& def){
  auto pos=s.find(key); if(pos==std::wstring::npos) return def;
  pos=s.find(L":",pos); if(pos==std::wstring::npos) return def;
  pos=s.find(L"\"",pos); if(pos==std::wstring::npos) return def; pos++;
  auto end=s.find(L"\"",pos); if(end==std::wstring::npos) return def;
  return s.substr(pos,end-pos);
}

bool loadConfigFromFile(const std::wstring& path, AppConfig& out, std::wstring& err){
  std::wstring s = readFileWide(path, err);
  if(s.empty()) return false;

  out.intervalSeconds = findInt(s, L"\"interval_seconds\"", out.intervalSeconds);
  out.timeoutSeconds  = findInt(s, L"\"timeout_seconds\"", out.timeoutSeconds);
  out.hostnameOverride = findString(s, L"\"hostname_override\"", out.hostnameOverride);

  out.metrics.cpu     = findBool(s, L"\"cpu\"", out.metrics.cpu);
  out.metrics.memory  = findBool(s, L"\"memory\"", out.metrics.memory);
  out.metrics.disk    = findBool(s, L"\"disk\"", out.metrics.disk);
  out.metrics.volumes = findBool(s, L"\"volumes\"", out.metrics.volumes);
  out.metrics.network = findBool(s, L"\"network\"", out.metrics.network);
  out.metrics.users   = findBool(s, L"\"users\"", out.metrics.users);

  out.json.enabled  = findBool(s, L"\"json\"", out.json.enabled);
  out.json.endpoint = findString(s, L"\"endpoint\"", out.json.endpoint);
  out.json.apiKey   = findString(s, L"\"api_key\"", out.json.apiKey);

  out.prom.enabled     = findBool(s, L"\"prometheus\"", out.prom.enabled);
  out.prom.pushgateway = findString(s, L"\"pushgateway\"", out.prom.pushgateway);
  out.prom.job         = findString(s, L"\"job\"", out.prom.job);

  out.otlp.enabled  = findBool(s, L"\"otlp\"", out.otlp.enabled);
  out.otlp.endpoint = findString(s, L"\"endpoint\"", out.otlp.endpoint);

  return true;
}
