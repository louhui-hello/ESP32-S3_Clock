<#
.SYNOPSIS
    把 ESP-IDF 工程的 build 产物合并成一个可从 0x0 烧录的 bin。

.EXAMPLE
    .\merge_bin.ps1 -ProjectDir ..\..\tools\i2c_scan\i2c_scan

.EXAMPLE
    .\merge_bin.ps1 -ProjectDir ..\..\firmware\test_cache\sample_project -OutFile demo.bin
#>
param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectDir,

    [string]$OutFile = "",

    [string]$Chip = "esp32s3",

    [string]$Esptool = "C:\Users\qwert\.platformio\penv\Scripts\esptool.exe"
)

$ErrorActionPreference = "Stop"

# 找 build/flash_args
$build = Join-Path $ProjectDir "build"
$argsFile = Join-Path $build "flash_args"

if (-not (Test-Path $argsFile)) {
    Write-Host "❌ 找不到 $argsFile" -ForegroundColor Red
    Write-Host "   先在 ESP-IDF 环境里跑一次 idf.py build" -ForegroundColor Yellow
    exit 1
}

# 解析 flash_args：跳过 -- 开头的行，其余是 "<偏移> <文件>"
$pairs = @()
Get-Content $argsFile | ForEach-Object {
    $line = $_.Trim()
    if ($line -eq "" -or $line.StartsWith("--")) { return }
    $parts = $line -split "\s+", 2
    if ($parts.Count -eq 2) {
        $pairs += $parts[0]
        $pairs += ($parts[1] -replace "/", "\")
    }
}

if ($pairs.Count -eq 0) {
    Write-Host "❌ flash_args 里没解析到任何 偏移/文件 对" -ForegroundColor Red
    exit 1
}

# 默认输出名：<工程目录名>_merged.bin
if ($OutFile -eq "") {
    $name = Split-Path $ProjectDir -Leaf
    $OutFile = Join-Path (Get-Location) "$name`_merged.bin"
}

if (-not (Test-Path $Esptool)) {
    Write-Host "❌ 找不到 esptool: $Esptool" -ForegroundColor Red
    Write-Host "   用 -Esptool 指定路径，或 pip install esptool" -ForegroundColor Yellow
    exit 1
}

Write-Host "工程: $ProjectDir" -ForegroundColor Cyan
Write-Host "输出: $OutFile" -ForegroundColor Cyan
Write-Host "分段: $($pairs.Count / 2) 个" -ForegroundColor Cyan

# 相对 build 目录执行，因为 flash_args 里的路径是相对 build 的
Push-Location $build
try {
    & $Esptool --chip $Chip merge-bin -o $OutFile @pairs
    if ($LASTEXITCODE -ne 0) { throw "esptool 返回 $LASTEXITCODE" }
}
finally {
    Pop-Location
}

if (Test-Path $OutFile) {
    $kb = [math]::Round((Get-Item $OutFile).Length / 1KB)
    Write-Host "" 
    Write-Host "✅ 合并完成: $OutFile  ($kb KB)" -ForegroundColor Green
    Write-Host "   烧录器偏移填 0x0" -ForegroundColor Green
}
