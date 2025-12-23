#define UNICODE
#include "http_client.h"
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

static std::wstring lastErrorToString(){
  DWORD err=GetLastError();
  wchar_t* buf=nullptr;
  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                 nullptr, err, 0, (LPWSTR)&buf, 0, nullptr);
  std::wstring s = buf ? buf : L"";
  if(buf) LocalFree(buf);
  return s;
}

HttpResponse httpPostUtf8(const std::wstring& url,const std::wstring& headers,const std::string& bodyUtf8,int timeoutSeconds){
  HttpResponse resp{};
  URL_COMPONENTS uc{}; uc.dwStructSize=sizeof(uc);
  wchar_t host[256]; wchar_t path[2048];
  uc.lpszHostName=host; uc.dwHostNameLength=256;
  uc.lpszUrlPath=path; uc.dwUrlPathLength=2048;
  if(!WinHttpCrackUrl(url.c_str(),0,0,&uc)){ resp.error=L"WinHttpCrackUrl failed: "+lastErrorToString(); return resp; }

  std::wstring hostW(host, uc.dwHostNameLength);
  std::wstring pathW(path, uc.dwUrlPathLength);

  HINTERNET hSession=WinHttpOpen(L"MetricsAgent/2.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if(!hSession){ resp.error=L"WinHttpOpen failed: "+lastErrorToString(); return resp; }
  WinHttpSetTimeouts(hSession, timeoutSeconds*1000, timeoutSeconds*1000, timeoutSeconds*1000, timeoutSeconds*1000);
  HINTERNET hConnect=WinHttpConnect(hSession, hostW.c_str(), uc.nPort, 0);
  if(!hConnect){ resp.error=L"WinHttpConnect failed: "+lastErrorToString(); WinHttpCloseHandle(hSession); return resp; }
  DWORD flags = (uc.nScheme==INTERNET_SCHEME_HTTPS)?WINHTTP_FLAG_SECURE:0;
  HINTERNET hReq=WinHttpOpenRequest(hConnect, L"POST", pathW.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
  if(!hReq){ resp.error=L"WinHttpOpenRequest failed: "+lastErrorToString(); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return resp; }

  BOOL ok=WinHttpSendRequest(hReq, headers.c_str(), (DWORD)headers.size(), (LPVOID)bodyUtf8.data(), (DWORD)bodyUtf8.size(), (DWORD)bodyUtf8.size(), 0);
  if(!ok){ resp.error=L"WinHttpSendRequest failed: "+lastErrorToString(); WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return resp; }
  ok=WinHttpReceiveResponse(hReq, nullptr);
  if(!ok){ resp.error=L"WinHttpReceiveResponse failed: "+lastErrorToString(); WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return resp; }

  DWORD status=0, len=sizeof(status);
  WinHttpQueryHeaders(hReq, WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &len, WINHTTP_NO_HEADER_INDEX);
  resp.status=status; resp.ok=(status>=200 && status<300);

  WinHttpCloseHandle(hReq); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
  return resp;
}
