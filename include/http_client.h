#pragma once
#include <string>
struct HttpResponse{bool ok=false; unsigned long status=0; std::wstring error;};
HttpResponse httpPostUtf8(const std::wstring& url,const std::wstring& headers,const std::string& bodyUtf8,int timeoutSeconds);
