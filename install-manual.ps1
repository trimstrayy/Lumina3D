# Manual dependency installer - Downloads pre-built libraries

Write-Host "====================================" -ForegroundColor Cyan
Write-Host "  Manual Dependency Installer" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

$LIB_DIR = Join-Path $PSScriptRoot "external"
$INCLUDE_DIR = Join-Path $LIB_DIR "include"
$BIN_DIR = Join-Path $LIB_DIR "bin"
$LIB_FILES_DIR = Join-Path $LIB_DIR "lib"

# Create directories
New-Item -ItemType Directory -Force -Path $INCLUDE_DIR | Out-Null
New-Item -ItemType Directory -Force -Path $BIN_DIR | Out-Null
New-Item -ItemType Directory -Force -Path $LIB_FILES_DIR | Out-Null

Write-Host "Downloading GLFW..." -ForegroundColor Yellow
$glfwUrl = "https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip"
$glfwZip = Join-Path $LIB_DIR "glfw.zip"

Invoke-WebRequest -Uri $glfwUrl -OutFile $glfwZip
Expand-Archive -Path $glfwZip -DestinationPath $LIB_DIR -Force

# Copy GLFW files
$glfwExtracted = Join-Path $LIB_DIR "glfw-3.4.bin.WIN64"
Copy-Item -Path (Join-Path $glfwExtracted "include\GLFW") -Destination $INCLUDE_DIR -Recurse -Force
Copy-Item -Path (Join-Path $glfwExtracted "lib-vc2022\glfw3.lib") -Destination $LIB_FILES_DIR -Force
Copy-Item -Path (Join-Path $glfwExtracted "lib-vc2022\glfw3dll.lib") -Destination $LIB_FILES_DIR -Force
Copy-Item -Path (Join-Path $glfwExtracted "lib-vc2022\glfw3.dll") -Destination $BIN_DIR -Force

Remove-Item $glfwZip -Force
Write-Host "GLFW installed successfully!" -ForegroundColor Green

Write-Host "Downloading GLM..." -ForegroundColor Yellow
$glmUrl = "https://github.com/g-truc/glm/releases/download/1.0.1/glm-1.0.1-light.zip"
$glmZip = Join-Path $LIB_DIR "glm.zip"

Invoke-WebRequest -Uri $glmUrl -OutFile $glmZip
Expand-Archive -Path $glmZip -DestinationPath $LIB_DIR -Force

# Copy GLM headers
$glmExtracted = Join-Path $LIB_DIR "glm"
if (Test-Path $glmExtracted) {
    Copy-Item -Path $glmExtracted -Destination $INCLUDE_DIR -Recurse -Force
}

Remove-Item $glmZip -Force
Write-Host "GLM installed successfully!" -ForegroundColor Green

Write-Host ""
Write-Host "====================================" -ForegroundColor Green
Write-Host "  Installation Complete!" -ForegroundColor Green
Write-Host "====================================" -ForegroundColor Green
Write-Host ""
Write-Host "Libraries installed to: $LIB_DIR" -ForegroundColor Yellow
Write-Host ""
Write-Host "Next: Update CMakeLists.txt to use these libraries" -ForegroundColor Yellow
