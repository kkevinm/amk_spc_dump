@echo off
setlocal EnableDelayedExpansion

:: Change to "0" to disable Yoshi drums in all the SPCs
set YOSHI_DRUMS="1"

:: Change to "0" to not generate SPCs of global songs (various jingles)
set GLOBAL_SONGS="0"

set /p ROM="ROM path: "
set ROM=!ROM:"=!
set OUT=!ROM:~0,-4!_spcs
set SONG_LENGTH_S="240"
set FADE_LENGTH_MS="10000"

if exist "!OUT!" (
    rmdir /s /q "!OUT!" > nul
)

mkdir "!OUT!" > nul

:: TODO remove .\build
.\build\amk_spc_dump.exe "!ROM!" "!OUT!" "!SONG_LENGTH_S!" "!FADE_LENGTH_MS!" "!YOSHI_DRUMS!" "!GLOBAL_SONGS!"

pause
