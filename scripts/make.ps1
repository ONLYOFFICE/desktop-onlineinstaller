param (
    [System.Version]$Version = "0.0.0.0",
    [string]$Arch = "x64",
    [string]$Target,
    [string]$CompanyName = "ONLYOFFICE",
    [string]$ProductName = "OnlineInstaller",
    [string]$SourceDir,
    [string]$BuildDir,
    [switch]$Sign,
    [string]$CertName = "Ascensio System SIA",
    [string]$TimestampServer = "http://timestamp.digicert.com"
)

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

if (-not $SourceDir) {
    $BuildPrefix = switch ($Arch) {
        "x64" { "win_64" }
        "x86" { "win_32" }
    }
    if ($Target -eq "xp") {
        $BuildPrefix += "_xp"
    }
    $SourceDir = "$PSScriptRoot\..\out\" `
        + "$BuildPrefix\$CompanyName\$ProductName" | Resolve-Path
}
if (-not (Test-Path "$SourceDir")) {
    Write-Error "Path `"$SourceDir`" does not exist"
}
if (-not $BuildDir) {
    $BuildDir = ".build.$Arch"
}

Write-Host @"
Version     = $Version
Arch        = $Arch
Target      = $Target
CompanyName = $CompanyName
ProductName = $ProductName
SourceDir   = $SourceDir
BuildDir    = $BuildDir
Sign        = $Sign
"@

####

Write-Host "`n[ Prepare build directory ]"

if (Test-Path "$BuildDir") {
    Write-Host "REMOVE DIR: $BuildDir"
    Remove-Item -Force -Recurse -LiteralPath "$BuildDir"
}

Write-Host "CREATE DIR: $BuildDir\desktop"
New-Item -ItemType Directory -Force -Path "$BuildDir\desktop" | Out-Null

Write-Host "COPY: $SourceDir\* > $BuildDir\desktop\"
Copy-Item -Force -Recurse `
    -Path "$SourceDir\*" `
    -Destination "$BuildDir\desktop\"

####

Write-Host "`n[ Sign files ]"

if ($Sign) {
    Set-Location "$BuildDir\desktop"
    $SignFiles = Get-ChildItem `
        *.exe `
        | Resolve-Path -Relative

    # Sign
    Write-Host "signtool sign /a /n $CertName /t $TimestampServer ..."
    & signtool sign /a /n $CertName /t $TimestampServer /v $SignFiles
    if ($LastExitCode -ne 0) { throw }

    # Verify
    Write-Host "signtool verify /q /pa /all ..."
    & signtool verify /q /pa /all $SignFiles | Out-Null
    if ($LastExitCode -ne 0) { throw }

    Set-Location $PSScriptRoot
}

if (Test-Path "$BuildDir\desktop\online-installer.exe") {
    $dst = "$PSScriptRoot\OnlineInstaller-$Version-$Arch" + $(if ($Target -eq "xp") {"-xp"}) + ".exe"
    Write-Host "MOVE: $BuildDir\desktop\online-installer.exe > $dst"
    Move-Item `
        -Path "$BuildDir\desktop\online-installer.exe" `
        -Destination $dst
}
