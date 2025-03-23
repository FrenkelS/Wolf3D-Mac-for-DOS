if "%WATCOM%" == "" goto error

mkdir WC32
wmake -f makefmy.w32 WC32\MWOLF386.EXE
del *.err
goto end

:error
@echo Set the environment variables before running this script!

:end
