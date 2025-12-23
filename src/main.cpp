#define UNICODE
#define _WIN32_WINNT 0x0601

#include <windows.h>
#include <shellapi.h>
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

// MinGW's default subsystem can be "windows" unless overridden, which expects WinMain.
// Provide a Unicode WinMain wrapper that forwards to wmain.
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int){
  int argc = 0;
  LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  int rc = wmain(argc, argv);
  if (argv) LocalFree(argv);
  return rc;
}


// Some CRT/startup combinations still look for ANSI WinMain. Provide it too.
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR, int nCmdShow){
  return wWinMain(hInst, hPrev, GetCommandLineW(), nCmdShow);
}
