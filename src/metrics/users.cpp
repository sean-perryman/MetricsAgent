#define UNICODE
#include <lm.h>
#include <vector>
#include <string>
#include "metrics.h"

#pragma comment(lib, "netapi32.lib")

std::vector<std::wstring> collectUsers() {
  std::vector<std::wstring> users;
  LPWKSTA_USER_INFO_1 buf = nullptr;
  DWORD read = 0, total = 0;

  if (NetWkstaUserEnum(nullptr, 1, (LPBYTE*)&buf, MAX_PREFERRED_LENGTH, &read, &total, nullptr) == NERR_Success) {
    for (DWORD i = 0; i < read; i++) users.push_back(buf[i].wkui1_username);
  }
  if (buf) NetApiBufferFree(buf);
  return users;
}
