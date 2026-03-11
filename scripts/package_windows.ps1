param(
    [string]$Version = "",
    [string]$BuildDir = "build",
    [string]$OutputDir = "",
    [string]$QtBinDir = "",
    [switch]$SkipInstaller,
    [switch]$EmbedReleaseKeys,
    [string]$DifyApiKey = "",
    [string]$TianxingApiKey = "",
    [string]$SupabaseUrl = "",
    [string]$SupabaseAnonKey = ""
)

$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptDir '..')
Set-Location $repoRoot

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Green
}

function Get-ProjectVersion {
    $cmakeFile = Join-Path $repoRoot 'CMakeLists.txt'
    $match = Select-String -Path $cmakeFile -Pattern 'project\(AILoginSystem VERSION ([0-9]+\.[0-9]+\.[0-9]+)'
    if (-not $match) {
        throw '无法从 CMakeLists.txt 读取项目版本号'
    }

    return $match.Matches[0].Groups[1].Value
}

function Resolve-Version {
    param([string]$RawVersion)

    if ([string]::IsNullOrWhiteSpace($RawVersion)) {
        return Get-ProjectVersion
    }

    if ($RawVersion.StartsWith('v')) {
        return $RawVersion.Substring(1)
    }

    return $RawVersion
}

function Ensure-Command {
    param([string]$Name)

    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "缺少命令: $Name"
    }
}

function Resolve-LocalPath {
    param([string]$PathValue)

    if ([System.IO.Path]::IsPathRooted($PathValue)) {
        return [System.IO.Path]::GetFullPath($PathValue)
    }

    return [System.IO.Path]::GetFullPath((Join-Path $repoRoot $PathValue))
}

function Escape-CppString {
    param([string]$Value)

    if ($null -eq $Value) {
        return ''
    }

    return $Value.Replace('\', '\\').Replace('"', '\"').Replace("`r", '').Replace("`n", '')
}

function Get-ResolvedSecretValue {
    param(
        [string]$ExplicitValue,
        [string]$EnvironmentKey
    )

    if (-not [string]::IsNullOrWhiteSpace($ExplicitValue)) {
        return $ExplicitValue
    }

    $envValue = [Environment]::GetEnvironmentVariable($EnvironmentKey)
    if ($null -eq $envValue) {
        return ''
    }

    return $envValue
}

function Ensure-EmbeddedKeys {
    param([bool]$ShouldEmbed)

    $embeddedKeysPath = Join-Path $repoRoot 'src/config/embedded_keys.h'
    if (-not $ShouldEmbed) {
        Write-Info '跳过内嵌密钥生成，保留现有 embedded_keys.h'
        return
    }

    $escapedDifyApiKey = Escape-CppString (Get-ResolvedSecretValue -ExplicitValue $DifyApiKey -EnvironmentKey 'DIFY_API_KEY')
    $escapedTianxingApiKey = Escape-CppString (Get-ResolvedSecretValue -ExplicitValue $TianxingApiKey -EnvironmentKey 'TIANXING_API_KEY')
    $escapedSupabaseUrl = Escape-CppString (Get-ResolvedSecretValue -ExplicitValue $SupabaseUrl -EnvironmentKey 'SUPABASE_URL')
    $escapedSupabaseAnonKey = Escape-CppString (Get-ResolvedSecretValue -ExplicitValue $SupabaseAnonKey -EnvironmentKey 'SUPABASE_ANON_KEY')

    $content = @"
#ifndef EMBEDDED_KEYS_H
#define EMBEDDED_KEYS_H

namespace EmbeddedKeys {
inline const char* DIFY_API_KEY = "$escapedDifyApiKey";
inline const char* TIANXING_API_KEY = "$escapedTianxingApiKey";
inline const char* SUPABASE_URL = "$escapedSupabaseUrl";
inline const char* SUPABASE_ANON_KEY = "$escapedSupabaseAnonKey";
}

#endif
"@

    Set-Content -Path $embeddedKeysPath -Value $content -Encoding UTF8
    Write-Info '已生成发布版 embedded_keys.h'
}

$resolvedVersion = Resolve-Version $Version
$resolvedBuildDir = Resolve-LocalPath $BuildDir
$resolvedOutputDir = if ([string]::IsNullOrWhiteSpace($OutputDir)) { $resolvedBuildDir } else { Resolve-LocalPath $OutputDir }
$deployDir = Join-Path $resolvedBuildDir 'deploy'
$zipFileName = "AI思政智慧课堂-Windows-x64-$resolvedVersion.zip"
$setupBaseName = "AI思政智慧课堂-Setup-Windows-x64-$resolvedVersion"
$zipPath = Join-Path $resolvedOutputDir $zipFileName
$setupPath = Join-Path $resolvedOutputDir "$setupBaseName.exe"
$resolvedQtBinDir = if (-not [string]::IsNullOrWhiteSpace($QtBinDir)) { Resolve-LocalPath $QtBinDir } else { $null }
$windeployqtCommand = if ($resolvedQtBinDir) { Join-Path $resolvedQtBinDir 'windeployqt.exe' } else { 'windeployqt' }

Write-Info "版本号: $resolvedVersion"
Write-Info "构建目录: $resolvedBuildDir"
Write-Info "产物目录: $resolvedOutputDir"

Ensure-Command 'cmake'
if ($resolvedQtBinDir) {
    if (-not (Test-Path $windeployqtCommand)) {
        throw "找不到 windeployqt: $windeployqtCommand"
    }
} else {
    Ensure-Command 'windeployqt'
}
if (-not $SkipInstaller) {
    Ensure-Command 'iscc'
}

Ensure-EmbeddedKeys -ShouldEmbed:$EmbedReleaseKeys.IsPresent

if (Test-Path $resolvedBuildDir) {
    Remove-Item -Path $resolvedBuildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $resolvedBuildDir | Out-Null
New-Item -ItemType Directory -Path $resolvedOutputDir -Force | Out-Null

Write-Info '配置 CMake Release 构建'
$cmakeConfigureArgs = @('-S', $repoRoot, '-B', $resolvedBuildDir, '-DCMAKE_BUILD_TYPE=Release')
if (Get-Command 'ninja' -ErrorAction SilentlyContinue) {
    $cmakeConfigureArgs += @('-G', 'Ninja')
}
cmake @cmakeConfigureArgs

Write-Info '编译应用'
cmake --build $resolvedBuildDir --config Release

$appExeCandidates = @(
    (Join-Path $resolvedBuildDir 'AILoginSystem.exe'),
    (Join-Path $resolvedBuildDir 'Release/AILoginSystem.exe')
)
$appExePath = $appExeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $appExePath) {
    throw "构建完成后未找到 AILoginSystem.exe"
}

if (Test-Path $deployDir) {
    Remove-Item -Path $deployDir -Recurse -Force
}
New-Item -ItemType Directory -Path $deployDir | Out-Null
Copy-Item -Path $appExePath -Destination $deployDir

Write-Info '执行 windeployqt'
$windeployArgs = @(
    (Join-Path $deployDir 'AILoginSystem.exe'),
    '--release',
    '--no-translations',
    '--no-opengl-sw'
)
if ($resolvedQtBinDir) {
    $windeployArgs += @('--qtpaths', (Join-Path $resolvedQtBinDir 'qtpaths.exe'))
}
& $windeployqtCommand @windeployArgs

Write-Info '复制运行时资源'
Copy-Item -Path (Join-Path $repoRoot 'resources/ppt') -Destination (Join-Path $deployDir 'ppt') -Recurse -Force
Copy-Item -Path (Join-Path $repoRoot 'resources/templates') -Destination (Join-Path $deployDir 'templates') -Recurse -Force
Copy-Item -Path (Join-Path $repoRoot 'resources/styles') -Destination (Join-Path $deployDir 'styles') -Recurse -Force

if (Test-Path $zipPath) {
    Remove-Item -Path $zipPath -Force
}

Write-Info "生成 ZIP: $zipFileName"
Compress-Archive -Path (Join-Path $deployDir '*') -DestinationPath $zipPath

if (-not $SkipInstaller) {
    if (Test-Path $setupPath) {
        Remove-Item -Path $setupPath -Force
    }

    Write-Info '生成安装包'
    $installerScript = Join-Path $scriptDir 'windows-installer.iss'
    iscc "/DMyAppVersion=$resolvedVersion" "/DMyOutputBaseFilename=$setupBaseName" "/DBuildRoot=$resolvedBuildDir" "/DOutputDir=$resolvedOutputDir" $installerScript
}

Write-Info '打包完成'
Write-Info "ZIP: $zipPath"
if (-not $SkipInstaller) {
    Write-Info "Setup: $setupPath"
}
