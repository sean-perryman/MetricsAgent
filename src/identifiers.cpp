#define UNICODE
#include "identifiers.h"
#include <windows.h>

std::wstring getHostname(const std::wstring& overrideName){
  if(!overrideName.empty()) return overrideName;
  wchar_t name[256]; DWORD size=256;
  if(GetComputerNameExW(ComputerNamePhysicalDnsHostname, name, &size)) return std::wstring(name, size);
  size=256;
  if(GetComputerNameW(name, &size)) return std::wstring(name, size);
  return L"unknown";
}

std::wstring getMachineGuid(){
  HKEY hKey;
  if(RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ|KEY_WOW64_64KEY, &hKey)!=ERROR_SUCCESS) return L"";
  wchar_t value[256]; DWORD size=sizeof(value), type=0;
  if(RegQueryValueExW(hKey, L"MachineGuid", nullptr, &type, (LPBYTE)value, &size)!=ERROR_SUCCESS){ RegCloseKey(hKey); return L""; }
  RegCloseKey(hKey);
  size_t chars = size/sizeof(wchar_t);
  if(chars && value[chars-1]==L'\0') chars--;
  return std::wstring(value, chars);
}
