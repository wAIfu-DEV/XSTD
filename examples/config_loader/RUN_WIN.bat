@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

SET OUTDIR=out
SET EXE=config_loader.exe

IF NOT EXIST %OUTDIR% (
    MKDIR %OUTDIR%
)

ECHO Building %EXE% with gcc...
gcc -std=c99 -Wall -Wextra -pedantic -g main.c -o %OUTDIR%\%EXE%
IF ERRORLEVEL 1 (
    ECHO [RUN] Build failed with gcc.
    EXIT /B 1
)

ECHO Running %EXE%...
%OUTDIR%\%EXE%
IF ERRORLEVEL 1 (
    ECHO [RUN] Execution failed with exit code %ERRORLEVEL%.
    EXIT /B %ERRORLEVEL%
)

ECHO [RUN] Completed successfully.
EXIT /B 0
