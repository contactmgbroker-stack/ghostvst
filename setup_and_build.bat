@echo off
chcp 65001 >nul
echo ============================================
echo   GhostSurf VST3 -- Installation Complete
echo ============================================
echo.
echo Ce script installe TOUT automatiquement :
echo   - Visual Studio Build Tools (compilateur C++)
echo   - CMake
echo   - Git
echo   - JUCE (telechargement automatique)
echo   - Compilation du plugin
echo   - Installation dans FL Studio
echo.
echo Duree estimee : 15-25 minutes (selon connexion)
echo.
pause

cd /d "%~dp0"

REM ═══════════════════════════════════════════════════════════════
REM  ETAPE 1 — Visual Studio Build Tools (compilateur C++ MSVC)
REM ═══════════════════════════════════════════════════════════════
echo.
echo [1/5] Verification du compilateur C++...

REM Check if MSVC is already installed via vswhere
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set MSBUILD_FOUND=0

if exist %VSWHERE% (
    for /f "tokens=*" %%i in ('%VSWHERE% -latest -requires Microsoft.VisualStudio.Workload.VCTools -property installationPath 2^>nul') do (
        if not "%%i"=="" set MSBUILD_FOUND=1
    )
)

if "%MSBUILD_FOUND%"=="1" (
    echo [OK] Compilateur C++ deja installe.
) else (
    echo [AUTO] Installation de Visual Studio Build Tools 2022...
    echo        (inclut le compilateur C++ MSVC -- environ 3-5 Go)
    echo        Cela peut prendre 10-15 minutes...
    echo.
    winget install --id Microsoft.VisualStudio.2022.BuildTools ^
        --override "--quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended" ^
        --accept-package-agreements --accept-source-agreements
    if errorlevel 1 (
        echo.
        echo [ALT] Telechargement direct des Build Tools...
        powershell -Command "Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vs_buildtools.exe' -OutFile 'vs_buildtools.exe'"
        vs_buildtools.exe --quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --wait
        del vs_buildtools.exe 2>nul
    )
    echo [OK] Compilateur installe.
)

REM ═══════════════════════════════════════════════════════════════
REM  ETAPE 2 — CMake
REM ═══════════════════════════════════════════════════════════════
echo.
echo [2/5] Verification de CMake...

where cmake >nul 2>&1
if errorlevel 1 (
    echo [AUTO] Installation de CMake...
    winget install --id Kitware.CMake -e --accept-package-agreements --accept-source-agreements
    if errorlevel 1 (
        powershell -Command "Invoke-WebRequest -Uri 'https://github.com/Kitware/CMake/releases/download/v3.31.7/cmake-3.31.7-windows-x86_64.msi' -OutFile 'cmake.msi'"
        msiexec /i cmake.msi /quiet ADD_CMAKE_TO_PATH=System
        del cmake.msi 2>nul
    )
    set "PATH=%PATH%;C:\Program Files\CMake\bin"
) else (
    echo [OK] CMake detecte.
)

REM ═══════════════════════════════════════════════════════════════
REM  ETAPE 3 — Git
REM ═══════════════════════════════════════════════════════════════
echo.
echo [3/5] Verification de Git...

where git >nul 2>&1
if errorlevel 1 (
    echo [AUTO] Installation de Git...
    winget install --id Git.Git -e --accept-package-agreements --accept-source-agreements
    set "PATH=%PATH%;C:\Program Files\Git\cmd"
) else (
    echo [OK] Git detecte.
)

REM ═══════════════════════════════════════════════════════════════
REM  ETAPE 4 — Clone JUCE
REM ═══════════════════════════════════════════════════════════════
echo.
echo [4/5] Telechargement de JUCE...

if not exist "JUCE\CMakeLists.txt" (
    git clone --depth=1 https://github.com/juce-framework/JUCE.git
    if errorlevel 1 (
        echo [ERREUR] Impossible de telecharger JUCE.
        echo Verifiez votre connexion internet et relancez le script.
        pause & exit /b 1
    )
    echo [OK] JUCE telecharge.
) else (
    echo [OK] JUCE deja present.
)

REM ═══════════════════════════════════════════════════════════════
REM  ETAPE 5 — Compilation
REM ═══════════════════════════════════════════════════════════════
echo.
echo [5/5] Compilation de GhostSurf...

if not exist build mkdir build

REM Try VS 2022, then 2019, then 2017
cmake -B build -G "Visual Studio 17 2022" -A x64 >nul 2>&1
if errorlevel 1 cmake -B build -G "Visual Studio 16 2019" -A x64 >nul 2>&1
if errorlevel 1 cmake -B build -G "Visual Studio 15 2017" -A x64 >nul 2>&1
if errorlevel 1 (
    echo [ERREUR] Configuration CMake echouee.
    echo Le compilateur C++ n'est peut-etre pas encore actif.
    echo Redemarrez Windows puis relancez ce script.
    pause & exit /b 1
)

cmake --build build --config Release
if errorlevel 1 (
    echo [ERREUR] Compilation echouee.
    echo Ouvrez build\GhostSurf.sln dans Visual Studio pour voir les details.
    pause & exit /b 1
)

REM ═══════════════════════════════════════════════════════════════
REM  INSTALLATION DANS FL STUDIO
REM ═══════════════════════════════════════════════════════════════
echo.
echo ============================================
echo   SUCCES ! GhostSurf compile !
echo ============================================
echo.

set VST3SRC=build\GhostSurf_artefacts\Release\VST3\GhostSurf.vst3
set VST3DST=C:\Program Files\Common Files\VST3\GhostSurf.vst3

set /p INST="Installer dans FL Studio maintenant ? (O/N) : "
if /i "%INST%"=="O" (
    xcopy /E /I /Y "%VST3SRC%" "%VST3DST%\"
    echo.
    echo [OK] GhostSurf installe !
    echo Dans FL Studio : Options ^> Manage Plugins ^> Chercher plugins installes
)

echo.
pause
