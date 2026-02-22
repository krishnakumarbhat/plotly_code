@echo off
REM Build the Singularity .simg image using WSL (Windows Subsystem for Linux).
REM Requires: WSL with Singularity installed (e.g., via Ubuntu + apt/singularity-ce).
REM Run this from the repo root: main_html\code\singularity\build_image.bat

setlocal
set "SCRIPT_DIR=%~dp0"
set "REPO_ROOT=%SCRIPT_DIR%..\..\.."

REM Convert Windows path to WSL path (e.g., C:\git\... -> /mnt/c/git/...)
for /f "usebackq tokens=*" %%i in (`wsl wslpath -u "%REPO_ROOT%"`) do set "WSL_REPO=%%i"

echo.
echo ============================================================
echo Building Singularity image via WSL
echo Repo (WSL path): %WSL_REPO%
echo ============================================================

REM Build requires sudo inside WSL; user may be prompted for password.
wsl bash -c "cd '%WSL_REPO%' && sudo singularity build --force main_html/code/singularity/log_viewer.simg main_html/code/singularity/log_viewer.def"

if %ERRORLEVEL% NEQ 0 (
    echo Build FAILED.
    exit /b 1
)

echo.
echo Build succeeded: main_html\code\singularity\log_viewer.simg
echo Copy this .simg file to your cluster and run via Slurm.
endlocal
