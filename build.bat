@echo off
setlocal

REM Kill any running instances of sql.exe
taskkill /F /IM sql.exe 2>nul

REM Set paths
set PATH=%PATH%;C:\WinFlexBison
set INCLUDE=%INCLUDE%;.

REM Clean up any existing files
if exist sql.exe del /F sql.exe
if exist *.obj del /F *.obj
if exist lex.yy.c del /F lex.yy.c
if exist parser.tab.c del /F parser.tab.c
if exist parser.tab.h del /F parser.tab.h

REM Initialize Visual Studio environment
call "D:\visual studio ²úÆ·\VC\Auxiliary\Build\vcvars32.bat"
if errorlevel 1 goto error

REM Generate lexer
win_flex --wincompat scanner.l
if errorlevel 1 goto error

REM Generate parser
win_bison -d parser.y
if errorlevel 1 goto error

REM Compile with additional options
cl /EHsc /W4 /wd4127 /wd4702 /std:c++17 /D_CRT_SECURE_NO_WARNINGS /DWIN32 /D_WINDOWS /I. main.cpp DBMS.cpp lex.yy.c parser.tab.c /Fe:sql_new.exe /TP
if errorlevel 1 goto error

REM Try to replace the old executable
if exist sql_new.exe (
    if exist sql.exe del /F sql.exe
    rename sql_new.exe sql.exe
    if errorlevel 1 goto error
)

REM Clean up intermediate files
del /F *.obj 2>nul
del /F lex.yy.c 2>nul
del /F parser.tab.c 2>nul
del /F parser.tab.h 2>nul

echo Build complete!
goto end

:error
echo Build failed!
if exist sql_new.exe del /F sql_new.exe

:end
endlocal
pause