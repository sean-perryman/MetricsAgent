param(
  [string]$InstallDir = "C:\Program Files\MetricsAgent",
  [string]$ExeName = "MetricsAgent.exe"
)

$exe = Join-Path $InstallDir $ExeName

if (!(Test-Path $exe)) {
  Write-Error "EXE not found at $exe"
  exit 1
}

sc.exe create MetricsAgent binPath= "`"$exe`"" start= auto
sc.exe start MetricsAgent
