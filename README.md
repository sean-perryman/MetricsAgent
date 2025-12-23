# MetricsAgent (v2)

Native Windows metrics agent (C++17) that runs as a Windows Service and can be **cross-compiled from Linux** using MinGW-w64.

## Build (Ubuntu 24.04+)
```bash
sudo apt-get update
sudo apt-get install -y cmake ninja-build mingw-w64

./scripts/build-linux.sh
```
Output: `build/MetricsAgent.exe`

If your filesystem has future timestamps (common after restores), Ninja may complain that `build.ninja` is “dirty”. Fix with:
```bash
find . -type f -newermt 'now+1min' -print -exec touch {} +
```

## Service install
```powershell
sc create MetricsAgent binPath= "C:\Program Files\MetricsAgent\MetricsAgent.exe"
sc start MetricsAgent
```

Foreground debug:
```powershell
MetricsAgent.exe --console --config C:\ProgramData\MetricsAgent\metrics-agent.json
```
