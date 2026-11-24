@echo off
setlocal
call "C:\APP\VS buildtools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 || exit /b 1
call "%~dp0build-windows.cmd"
exit /b %errorlevel%
