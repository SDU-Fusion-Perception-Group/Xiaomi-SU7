@echo off
setlocal
cd /d "%~dp0" || exit /b 1
if not defined CARGO_HOME if exist "C:\Sdk\Rust\cargo\bin\cargo.exe" set "CARGO_HOME=C:\Sdk\Rust\cargo"
if not defined RUSTUP_HOME if exist "C:\Sdk\Rust\rustup\toolchains\" set "RUSTUP_HOME=C:\Sdk\Rust\rustup"
if defined CARGO_HOME set "PATH=%CARGO_HOME%\bin;%PATH%"
if exist "C:\Sdk\Node\node.exe" set "PATH=C:\Sdk\Node;%PATH%"
if not defined STATION_VCVARS64 if exist "C:\Sdk\MSVC\VC\Auxiliary\Build\vcvars64.bat" set "STATION_VCVARS64=C:\Sdk\MSVC\VC\Auxiliary\Build\vcvars64.bat"
if defined STATION_VCVARS64 call "%STATION_VCVARS64%" || exit /b 1
if /i not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
  echo MSVC x64 environment is missing. Run from an x64 Native Tools Command Prompt or set STATION_VCVARS64. 1>&2
  exit /b 1
)
if not defined WindowsSDKVersion (
  echo Windows SDK environment is missing. 1>&2
  exit /b 1
)
if not defined CARGO_NET_OFFLINE set "CARGO_NET_OFFLINE=true"
node --test tests/*.test.mjs || exit /b 1
call npm run tauri build -- --no-bundle --target x86_64-pc-windows-msvc -- --locked || exit /b 1
set "RELEASE_ROOT=%~dp0src-tauri\target"
if defined CARGO_TARGET_DIR set "RELEASE_ROOT=%CARGO_TARGET_DIR%"
set "RELEASE_EXE=%RELEASE_ROOT%\x86_64-pc-windows-msvc\release\car-ground-station-v2.exe"
if not exist "%RELEASE_EXE%" exit /b 1
node tools/verify-windows-exe.mjs "%RELEASE_EXE%" || exit /b 1
exit /b 0
