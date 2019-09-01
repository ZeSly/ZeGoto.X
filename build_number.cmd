REM echo off
setlocal enabledelayedexpansion

set version_major=0
set version_minor=3
set buildnum=

for /f "tokens=3" %%i in (build_number.h) do set buildnum=%%i

if not exist release (
    mkdir release
)

copy dist\default\production\ZeGoto.X.production.hex release\
move release\ZeGoto.X.production.hex release\ZeGoto_%version_major%_%version_minor%_%buildnum%.hex

set /a buildnum=%buildnum% + 1

@echo #define VERSION "%version_major%.%version_minor%.%buildnum%" > build_number.h
@echo #define VERSION_MAJOR %version_major% >> build_number.h
@echo #define VERSION_MINOR %version_minor% >> build_number.h
@echo #define BUILD_NUMBER %buildnum% >> build_number.h
