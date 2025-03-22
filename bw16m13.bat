if "%WATCOM%" == "" goto error

mkdir WC16
wmake -f makefm13.w16 WC16\MACWOLF.EXE
del *.err
goto end

:error
@echo Set the environment variables before running this script!

:end
