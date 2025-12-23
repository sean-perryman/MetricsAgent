#define UNICODE
#define _WIN32_WINNT 0x0601

#include "service.h"
#include "agent.h"
#include "config.h"
#include <windows.h>
#include <thread>

static SERVICE_STATUS gStatus{};
static SERVICE_STATUS_HANDLE gHandle = nullptr;
static HANDLE gStopEvent = nullptr;
static std::wstring gCfgPath;

static void setState(DWORD state) {
  gStatus.dwCurrentState = state;
  SetServiceStatus(gHandle, &gStatus);
}

static void WINAPI ctrlHandler(DWORD code) {
  if (code == SERVICE_CONTROL_STOP) {
    setState(SERVICE_STOP_PENDING);
    if (gStopEvent) SetEvent(gStopEvent);
  }
}

static void WINAPI serviceMain(DWORD, LPWSTR*) {
  gHandle = RegisterServiceCtrlHandlerW(L"MetricsAgent", ctrlHandler);
  if (!gHandle) return;

  gStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  gStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  gStatus.dwWin32ExitCode = 0;
  gStatus.dwServiceSpecificExitCode = 0;
  gStatus.dwCheckPoint = 0;
  gStatus.dwWaitHint = 0;

  setState(SERVICE_START_PENDING);
  gStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

  AppConfig cfg{};
  std::wstring err;
  if (!loadConfigFromFile(gCfgPath, cfg, err)) {
    setState(SERVICE_STOPPED);
    return;
  }

  setState(SERVICE_RUNNING);

  std::thread t([&](){ runAgentLoop(cfg); });
  t.detach();

  WaitForSingleObject(gStopEvent, INFINITE);
  setState(SERVICE_STOPPED);
}

int runConsole(const std::wstring& configPath) {
  AppConfig cfg{};
  std::wstring err;
  if (!loadConfigFromFile(configPath, cfg, err)) return 2;
  runAgentLoop(cfg);
  return 0;
}

int runService(const std::wstring& configPath) {
  gCfgPath = configPath;
  SERVICE_TABLE_ENTRYW table[] = {
    { (LPWSTR)L"MetricsAgent", (LPSERVICE_MAIN_FUNCTIONW)serviceMain },
    { nullptr, nullptr }
  };
  if (!StartServiceCtrlDispatcherW(table)) {
    // If launched directly without SCM, fall back to console mode.
    return runConsole(configPath);
  }
  return 0;
}
