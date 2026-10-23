::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: - $Revision: 1.3 $
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
setlocal enabledelayedexpansion

:: ƒƒCƒ“ˆ—iƒRƒ}ƒ“ƒh‚ğŠÖ”‚ÉŠÛ“Š‚°‚·‚é‚¾‚¯j
call :sh_exec cmake ../..
call :sh_exec perl ../parse_config_h.pl --input config.h --output config.ini
FOR /F "tokens=1* delims== eol=;" %%I IN (config.ini) DO SET "%%I=%%~J"
SET proj_name_ver=%PROJECT_NAME%-%PROJECT_VERSION%
:: call :sh_exec echo %proj_name_ver%
call :sh_exec cmake --build . --config Release
call :sh_exec ctest -C Release

echo ‚·‚×‚Ä‚Ìˆ—‚ª³í‚ÉŠ®—¹‚µ‚Ü‚µ‚½B
exit /b 0

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: ‹¤’ÊŠÖ”: ƒRƒ}ƒ“ƒh‚ğÀs‚µAƒGƒ‰[‚È‚ç‘¦À‚ÉI—¹ƒR[ƒh1‚Å—‚¿‚é
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:sh_exec
echo.
echo [33m-$ [0m%*
%*
if %ERRORLEVEL% NEQ 0 (
    echo [31mError: Command failed with exit code %ERRORLEVEL%[0m
    exit 1
)
goto :eof
