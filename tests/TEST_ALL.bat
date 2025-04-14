@ECHO OFF

SET STD=c99
SET OPT=0
SET COMPILER=gcc.exe

::==============================================================================
:: TEST ALL COMPILERS
::==============================================================================
SET STD=c99
CALL :TestAllOpt

SET STD=c17
CALL :TestAllOpt

CALL :Terminate

::==============================================================================
:: FUNCTIONS
::==============================================================================

:TestAllOpt

SET OPT=0
CALL :TestAllComps

SET OPT=1
CALL :TestAllComps

SET OPT=2
CALL :TestAllComps

SET OPT=3
CALL :TestAllComps

SET OPT=s
CALL :TestAllComps

SET OPT=fast
CALL :TestAllComps

EXIT /b

:TestAllComps

SET COMPILER=gcc.exe
CALL :Test

SET COMPILER=clang.exe
CALL :Test

SET COMPILER=zig.exe cc
CALL :Test

EXIT /b

:Test

IF EXIST out/text.exe (
    DEL out/test.exe
)

IF EXIST out/text.ilk (
    DEL out/test.ilk
)

IF EXIST out/text.pdb (
    DEL out/test.pdb
)

ECHO Building with %COMPILER% %STD% -O%OPT%...

%COMPILER% -g test.c -o out/test.exe -std=%STD% -O%OPT% -Wall -Werror -pedantic

IF %ERRORLEVEL% EQU 0 (
    powershell -Command "Write-Host Successfully built with %COMPILER% %STD% -O%OPT% -ForegroundColor Green"
) ELSE (
    powershell -Command "Write-Host Failed to build with %COMPILER% %STD% -O%OPT% -ForegroundColor Red"
    CALL :Terminate
)

CD out
test.exe

IF %ERRORLEVEL% EQU 0 (
    powershell -Command "Write-Host Successfully passed all tests with %COMPILER% %STD% -O%OPT% -ForegroundColor Green"
) ELSE (
    powershell -Command "Write-Host Failed to pass all tests with %COMPILER% %STD% -O%OPT% -ForegroundColor Red"
    CALL :Terminate
)

CD ..

ECHO.

EXIT /b

:Terminate

IF EXIST out/text.exe (
    DEL out/test.exe
)

IF EXIST out/text.ilk (
    DEL out/test.ilk
)

IF EXIST out/text.pdb (
    DEL out/test.pdb
)

PAUSE
EXIT 0
