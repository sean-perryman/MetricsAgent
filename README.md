# MetricsAgent

A native Windows **system metrics agent** written in C++ that runs as a **Windows Service** and can be **cross-compiled from Linux** (MinGW-w64).

## Metrics
- CPU utilization & idle
- Memory used/total/free
- Disk performance (per PhysicalDisk instance)
  - reads/sec, writes/sec
  - avg queue length
  - read/write latency (ms)
  - utilization (% disk time)
  - **integrated totals** (monotonic) for reads/writes (approx)
- Volume capacity (per logical drive)
  - total/free bytes + free%
- Network performance (per interface)
  - bytes/sec
  - **integrated total** (monotonic) bytes (approx)
- Logged-in users

## Exporters
- JSON HTTP POST (custom endpoint)
- Prometheus text (Pushgateway POST)
- OpenTelemetry OTLP/HTTP (JSON payload to `/v1/metrics`)

## Build (Linux → Windows)
```bash
sudo apt-get update
sudo apt-get install -y cmake mingw-w64 ninja-build

./scripts/build-linux.sh
```
Artifacts:
- `build/MetricsAgent.exe`

## Install as a Windows Service
Copy `MetricsAgent.exe` and `config/metrics-agent.json` to the target.

```powershell
sc create MetricsAgent binPath= "C:\Program Files\MetricsAgent\MetricsAgent.exe"
sc start MetricsAgent
```

> Tip: run `MetricsAgent.exe --console --config C:\ProgramData\MetricsAgent\metrics-agent.json` for foreground debugging.

## Config
See `config/metrics-agent.json`.

## CI
GitHub Actions workflow at `.github/workflows/build.yml` builds Windows artifacts from Ubuntu.
