::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: - $Revision: 1.2 $
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
setlocal enabledelayedexpansion

:: ƒƒCƒ“ˆ—iƒRƒ}ƒ“ƒh‚ğŠÖ”‚ÉŠÛ“Š‚°‚·‚é‚¾‚¯j
call :sh_exec cmake ../..
call :sh_exec perl ../parse_config_h.pl
call :sh_exec cmake --build . --config Release
call :sh_exec ctest -C Release

echo ‚·‚×‚Ä‚Ìˆ—‚ª³í‚ÉŠ®—¹‚µ‚Ü‚µ‚½B
exit /b 0

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: ‹¤’ÊŠÖ”: ƒRƒ}ƒ“ƒh‚ğÀs‚µAƒGƒ‰[‚È‚ç‘¦À‚ÉI—¹ƒR[ƒh1‚Å—‚¿‚é
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:sh_exec
echo.
echo [33m$ [0m%*
%*
if %ERRORLEVEL% NEQ 0 (
    echo [31mError: Command failed with exit code %ERRORLEVEL%[0m
    exit 1
)
goto :eof
