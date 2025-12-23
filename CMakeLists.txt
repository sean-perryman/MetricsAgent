cmake_minimum_required(VERSION 3.20)
project(MetricsAgent LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MetricsAgent
  src/main.cpp
  src/agent.cpp
  src/config.cpp
  src/identifiers.cpp
  src/http_client.cpp
  src/service.cpp
  src/metrics/cpu.cpp
  src/metrics/memory.cpp
  src/metrics/disk.cpp
  src/metrics/network.cpp
  src/metrics/users.cpp
  src/exporters/exporter_json.cpp
  src/exporters/exporter_prometheus.cpp
  src/exporters/exporter_otlp.cpp
)

target_include_directories(MetricsAgent PRIVATE include)
target_link_libraries(MetricsAgent PRIVATE winhttp pdh netapi32 advapi32)
