param(
    [string]$Version = "",
    [string]$BuildDir = "build",
    [string]$OutputDir = "",
    [string]$QtBinDir = "",
    [string]$CMakeToolchainFile = "",
    [switch]$SkipInstaller,
    [switch]$EmbedReleaseKeys,
    [string]$DifyApiKey = "",
    [string]$MinimaxApiKey = "",
    [string]$MinimaxBaseUrl = "",
    [string]$MinimaxModel = "",
    [string]$TianxingApiKey = "",
    [string]$SupabaseUrl = "",
    [string]$SupabaseAnonKey = "",
    [string]$ZhipuApiKey = "",
    [string]$ZhipuBaseUrl = ""
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

function Ensure-ConfigEnv {
    param([bool]$ShouldEmbed)

    if (-not $ShouldEmbed) {
        Write-Info '跳过密钥导出，不生成 config.env'
        return
    }

    $configEnvPath = Join-Path $deployDir 'config.env'
    $difyApiKey = Get-ResolvedSecretValue -ExplicitValue $DifyApiKey -EnvironmentKey 'DIFY_API_KEY'
    $minimaxApiKey = Get-ResolvedSecretValue -ExplicitValue $MinimaxApiKey -EnvironmentKey 'MINIMAX_API_KEY'
    $minimaxBaseUrl = Get-ResolvedSecretValue -ExplicitValue $MinimaxBaseUrl -EnvironmentKey 'MINIMAX_API_BASE_URL'
    $minimaxModel = Get-ResolvedSecretValue -ExplicitValue $MinimaxModel -EnvironmentKey 'MINIMAX_MODEL'
    $tianxingApiKey = Get-ResolvedSecretValue -ExplicitValue $TianxingApiKey -EnvironmentKey 'TIANXING_API_KEY'
    $supabaseUrl = Get-ResolvedSecretValue -ExplicitValue $SupabaseUrl -EnvironmentKey 'SUPABASE_URL'
    $supabaseAnonKey = Get-ResolvedSecretValue -ExplicitValue $SupabaseAnonKey -EnvironmentKey 'SUPABASE_ANON_KEY'
    $zhipuApiKey = Get-ResolvedSecretValue -ExplicitValue $ZhipuApiKey -EnvironmentKey 'ZHIPU_API_KEY'
    $zhipuBaseUrl = Get-ResolvedSecretValue -ExplicitValue $ZhipuBaseUrl -EnvironmentKey 'ZHIPU_BASE_URL'

    $content = @"
# AI 思政智慧课堂 - 运行时配置
# 此文件包含 API 密钥，请勿公开分享

DIFY_API_KEY=$difyApiKey
MINIMAX_API_KEY=$minimaxApiKey
MINIMAX_API_BASE_URL=$(if ([string]::IsNullOrWhiteSpace($minimaxBaseUrl)) { 'https://api.minimaxi.com/v1' } else { $minimaxBaseUrl })
MINIMAX_MODEL=$(if ([string]::IsNullOrWhiteSpace($minimaxModel)) { 'MiniMax-M2.7' } else { $minimaxModel })
TIANXING_API_KEY=$tianxingApiKey
SUPABASE_URL=$supabaseUrl
SUPABASE_ANON_KEY=$supabaseAnonKey
ZHIPU_API_KEY=$zhipuApiKey
ZHIPU_BASE_URL=$(if ([string]::IsNullOrWhiteSpace($zhipuBaseUrl)) { 'https://open.bigmodel.cn/api/coding/paas/v4' } else { $zhipuBaseUrl })
"@

    Set-Content -Path $configEnvPath -Value $content -Encoding UTF8
    Write-Info '已生成发布版 config.env'
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

if (Test-Path $resolvedBuildDir) {
    Remove-Item -Path $resolvedBuildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $resolvedBuildDir | Out-Null
New-Item -ItemType Directory -Path $resolvedOutputDir -Force | Out-Null

Write-Info '配置 CMake Release 构建'
$cmakeConfigureArgs = @('-S', $repoRoot, '-B', $resolvedBuildDir, '-DCMAKE_BUILD_TYPE=Release')
if ($resolvedQtBinDir) {
    $qtRootDir = Split-Path -Parent $resolvedQtBinDir
    $cmakeConfigureArgs += @("-DCMAKE_PREFIX_PATH=$qtRootDir")
}
if (-not [string]::IsNullOrWhiteSpace($CMakeToolchainFile)) {
    $resolvedToolchainFile = Resolve-LocalPath $CMakeToolchainFile
    $cmakeConfigureArgs += @("-DCMAKE_TOOLCHAIN_FILE=$resolvedToolchainFile")
}
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

# 生成运行时配置文件（密钥随包分发）
Ensure-ConfigEnv -ShouldEmbed:$EmbedReleaseKeys.IsPresent

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
