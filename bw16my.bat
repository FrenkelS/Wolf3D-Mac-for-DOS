if "%WATCOM%" == "" goto error

mkdir WC16
wmake -f makefmy.w16 WC16\MACWOLF.EXE
del *.err
goto end

:error
@echo Set the environment variables before running this script!

:end
