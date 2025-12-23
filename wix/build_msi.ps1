param(
  [string]$WixBin = "C:\Program Files (x86)\WiX Toolset v3.11\bin",
  [string]$OutMsi = "MetricsAgent.msi"
)

$Candle = Join-Path $WixBin "candle.exe"
$Light  = Join-Path $WixBin "light.exe"

& $Candle "MetricsAgent.wxs"
& $Light "MetricsAgent.wixobj" -o $OutMsi
