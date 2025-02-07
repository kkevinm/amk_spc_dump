@echo off
setlocal EnableDelayedExpansion

:: Change to "0" to disable Yoshi drums in all the SPCs
set YOSHI_DRUMS="1"

:: Change to "0" to not generate SPCs of global songs (various jingles)
set GLOBAL_SONGS="0"

:: Change to "1" to generate the first 31 local song SPCs, which are usually just the SMW vanilla songs
set VANILLA_SONGS="0"

:: Song length set for all generated SPCs (in seconds)
set SONG_LENGTH_S="240"

:: Song fadeout length set for all generated SPCs (in milliseconds)
set FADE_LENGTH_MS="10000"

:: Override how many global songs are in the ROM (0 = don't override). In most cases you want to keep this at 0.
set GLOBAL_SONGS_NUMBER_OVERRIDE="0"

:: Don't change from here if you don't know what you're doing
set /p ROM="ROM path: "
set ROM=!ROM:"=!
set OUT=!ROM:~0,-4!_spcs

if exist "!OUT!" (
    rmdir /s /q "!OUT!" > nul
)

mkdir "!OUT!" > nul

.\build\amk_spc_dump.exe "!ROM!" "!OUT!" "!SONG_LENGTH_S!" "!FADE_LENGTH_MS!" "!YOSHI_DRUMS!" "!GLOBAL_SONGS!" "!VANILLA_SONGS!" "!GLOBAL_SONGS_NUMBER_OVERRIDE!"

pause
