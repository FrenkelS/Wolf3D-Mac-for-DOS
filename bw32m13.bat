if "%WATCOM%" == "" goto error

mkdir WC32
wmake -f makefm13.w32 WC32\MACWOLF.EXE
del *.err
goto end

:error
@echo Set the environment variables before running this script!

:end
