#define UNICODE
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <string>
#include "service.h"

static std::wstring getArgValue(int argc, wchar_t** argv, const std::wstring& key, const std::wstring& def){
  for(int i=1;i<argc;i++){ if(key==argv[i] && i+1<argc) return argv[i+1]; }
  return def;
}
static bool hasArg(int argc, wchar_t** argv, const std::wstring& key){
  for(int i=1;i<argc;i++) if(key==argv[i]) return true;
  return false;
}
int wmain(int argc, wchar_t** argv){
  std::wstring cfg = getArgValue(argc, argv, L"--config", L"config\\metrics-agent.json");
  if(hasArg(argc, argv, L"--console")) return runConsole(cfg);
  return runService(cfg);
}
