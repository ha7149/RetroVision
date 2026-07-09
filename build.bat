@echo off
SETLOCAL EnableDelayedExpansion

echo ===================================================
echo RetroVision Native Windows Build Automation Pipeline
echo ===================================================

:: 1. Force native Windows Build Tools and CMake to take path priority
set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\Common Extensions\Microsoft\CMake\CMake\bin;C:\Program Files\CMake\bin;%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\system32\Wbem;%SystemRoot%\System32\WindowsPowerShell\v1.0\"

:: 2. Track down workspace vcpkg dependency path
if exist "%~dp0..\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_ROOT=%~dp0..\vcpkg"
    echo [INFO] Detected vcpkg side-by-side at: !VCPKG_ROOT!
) else (
    echo [ERROR] Could not locate the 'vcpkg' folder side-by-side with this repository.
    goto :error
)

:: 3. Sync vcpkg recipes with current toolchain requirements
echo [INFO] Syncing vcpkg recipes...
pushd "!VCPKG_ROOT!"
git pull --quiet
if %errorlevel% neq 0 (
    echo [WARNING] Failed to pull latest vcpkg updates. Proceeding with local tree...
) else (
    call bootstrap-vcpkg.bat -disableMetrics >nul 2>&1
)
popd

set "BUILD_DIR=%~dp0build_windows"

:: 4. Clean up old build folder to prevent cache conflicts
if exist "%BUILD_DIR%" (
    echo [INFO] Wiping existing build folder...
    rmdir /s /q "%BUILD_DIR%"
)

echo [INFO] Creating build directory...
mkdir "%BUILD_DIR%"

echo [INFO] Configuring CMake with Native Windows Toolchain...
cmake -G "Visual Studio 18 2026" -A x64 ^
 -S "%~dp0." ^
 -B "%BUILD_DIR%" ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"

if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed.
    goto :error
)

echo [INFO] Executing Native C++ Compilation...
cmake --build "%BUILD_DIR%" --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    goto :error
)

echo ===================================================
echo [SUCCESS] RetroVision Windows Build Finished.
echo Run application using: \build_windows\Release\RetroVision.exe
echo ===================================================
pause
exit /b 0

:error
pause
exit /b 1