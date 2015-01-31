echo off
setlocal enabledelayedexpansion
set buildnum=

for /f "tokens=3" %%i in (build_number.h) do set buildnum=%%i

set /a buildnum=%buildnum% + 1

@echo #define BUILD_NUMBER "%buildnum%" > build_number.h