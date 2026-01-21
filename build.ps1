# Build script for Lumina3D Engine
# Run this script to build the project

$BUILD_DIR = "build"
$EXTERNAL_DIR = "external"

Write-Host "====================================" -ForegroundColor Cyan
Write-Host "  Lumina3D Engine Build Script" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

# Check if dependencies are installed
if (-not (Test-Path $EXTERNAL_DIR)) {
    Write-Host "WARNING: Dependencies not found!" -ForegroundColor Red
    Write-Host "Please run .\install-manual.ps1 first to install GLFW and GLM" -ForegroundColor Yellow
    Write-Host ""
    $response = Read-Host "Do you want to install dependencies now? (Y/N)"
    if ($response -eq "Y" -or $response -eq "y") {
        Write-Host "Installing dependencies..." -ForegroundColor Yellow
        & ".\install-manual.ps1"
        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: Dependency installation failed!" -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "Aborting build. Please install dependencies first." -ForegroundColor Red
        exit 1
    }
}

# Check if CMake is installed
Write-Host "Checking for CMake..." -ForegroundColor Yellow
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Host "ERROR: CMake not found. Please install CMake first." -ForegroundColor Red
    exit 1
}
Write-Host "CMake found: $($cmake.Version)" -ForegroundColor Green

# Create build directory
if (-not (Test-Path $BUILD_DIR)) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
}

# Configure with CMake
Write-Host ""
Write-Host "Configuring project with CMake..." -ForegroundColor Yellow
cmake -B $BUILD_DIR -G "MinGW Makefiles"

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: CMake configuration failed!" -ForegroundColor Red
    Write-Host "Make sure you have a C++ compiler installed (MinGW-w64 recommended)" -ForegroundColor Red
    Set-Location ..
    exit 1
}

# Build the project
Write-Host ""
Write-Host "Building project..." -ForegroundColor Yellow
cmake --build $BUILD_DIR --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: Build failed!" -ForegroundColor Red
    exit 1
}

# Success
Write-Host ""
Write-Host "====================================" -ForegroundColor Green
Write-Host "  Build Successful!" -ForegroundColor Green
Write-Host "====================================" -ForegroundColor Green
Write-Host ""
Write-Host "Executable location:" -ForegroundColor Yellow
Write-Host "  .\build\bin\Lumina3D.exe" -ForegroundColor Cyan
Write-Host ""
Write-Host "To run the engine:" -ForegroundColor Yellow
Write-Host "  cd build\bin" -ForegroundColor Cyan
Write-Host "  .\Lumina3D.exe" -ForegroundColor Cyan
Write-Host ""
