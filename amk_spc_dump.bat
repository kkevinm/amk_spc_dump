@echo off
setlocal EnableDelayedExpansion

:: TODO change back
::set /p ROM="ROM path: "
set ROM="beans.smc"
set ROM=!ROM:"=!
set OUT=!ROM:~0,-4!_spcs

if exist "!OUT!" (
    rmdir /s /q "!OUT!" > nul
)

mkdir "!OUT!" > nul

:: TODO remove .\build
.\build\amk_spc_dump.exe "!ROM!" "!OUT!"

:: TODO restore
::pause
