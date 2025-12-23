#pragma once
#include <string>

std::wstring getHostname(const std::wstring& overrideName);
std::wstring getMachineGuid(); // stable unique id
