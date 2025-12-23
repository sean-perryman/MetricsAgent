#define UNICODE
#include <windows.h>
#include <lm.h>
#include <vector>
#include <string>
#include "metrics.h"
#pragma comment(lib,"netapi32.lib")
std::vector<std::wstring> collectUsers(){ std::vector<std::wstring> u; LPWKSTA_USER_INFO_1 buf=nullptr; DWORD r=0,t=0; if(NetWkstaUserEnum(nullptr,1,(LPBYTE*)&buf,MAX_PREFERRED_LENGTH,&r,&t,nullptr)==NERR_Success){ for(DWORD i=0;i<r;i++) u.push_back(buf[i].wkui1_username);} if(buf) NetApiBufferFree(buf); return u; }
