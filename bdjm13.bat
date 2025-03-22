if "%DJDIR%" == "" goto error

mkdir DJ

nasm a_mv_mix.asm -f coff

set CFLAGS=-Ofast -march=i386 -flto -fwhole-program -fomit-frame-pointer -funroll-loops -fgcse-sm -fgcse-las -fipa-pta -mpreferred-stack-boundary=2 -Wno-attributes -Wpedantic
@rem set CFLAGS=%CFLAGS% -Wall -Wextra

@set GLOBOBJS=
@set GLOBOBJS=%GLOBOBJS% a_al_mid.c
@set GLOBOBJS=%GLOBOBJS% a_blast.c
@set GLOBOBJS=%GLOBOBJS% a_dma.c
@set GLOBOBJS=%GLOBOBJS% a_ll_man.c
@set GLOBOBJS=%GLOBOBJS% a_midi.c
@set GLOBOBJS=%GLOBOBJS% a_mpu401.c
@set GLOBOBJS=%GLOBOBJS% a_multiv.c
@set GLOBOBJS=%GLOBOBJS% a_music.c
@set GLOBOBJS=%GLOBOBJS% a_mv_mix.o
@set GLOBOBJS=%GLOBOBJS% a_pcfx.c
@set GLOBOBJS=%GLOBOBJS% a_taskmn.c
@set GLOBOBJS=%GLOBOBJS% Data.c
@set GLOBOBJS=%GLOBOBJS% Doors.c
@set GLOBOBJS=%GLOBOBJS% EnMove.c
@set GLOBOBJS=%GLOBOBJS% EnThink.c
@set GLOBOBJS=%GLOBOBJS% i_aud32.c
@set GLOBOBJS=%GLOBOBJS% i_input.c
@set GLOBOBJS=%GLOBOBJS% i_main.c
@set GLOBOBJS=%GLOBOBJS% i_pal256.c
@set GLOBOBJS=%GLOBOBJS% i_timer.c
@set GLOBOBJS=%GLOBOBJS% i_vm13.c
@set GLOBOBJS=%GLOBOBJS% InterMis.c
@set GLOBOBJS=%GLOBOBJS% Intro.c
@set GLOBOBJS=%GLOBOBJS% Level.c
@set GLOBOBJS=%GLOBOBJS% Missiles.c
@set GLOBOBJS=%GLOBOBJS% Music.c
@set GLOBOBJS=%GLOBOBJS% PlMove.c
@set GLOBOBJS=%GLOBOBJS% PlStuff.c
@set GLOBOBJS=%GLOBOBJS% PlThink.c
@set GLOBOBJS=%GLOBOBJS% PushWall.c
@set GLOBOBJS=%GLOBOBJS% RefBsp.c
@set GLOBOBJS=%GLOBOBJS% Refresh.c
@set GLOBOBJS=%GLOBOBJS% Refresh2.c
@set GLOBOBJS=%GLOBOBJS% RefSprite.c
@set GLOBOBJS=%GLOBOBJS% Sight.c
@set GLOBOBJS=%GLOBOBJS% SnesMain.c
@set GLOBOBJS=%GLOBOBJS% StateDef.c
@set GLOBOBJS=%GLOBOBJS% WolfIO.c
@set GLOBOBJS=%GLOBOBJS% WolfMain.c
@set GLOBOBJS=%GLOBOBJS% w_wad.c
@set GLOBOBJS=%GLOBOBJS% z_zone.c

gcc %GLOBOBJS% %CFLAGS% -o DJ/MACWOLF.EXE
strip -s DJ/MACWOLF.EXE
stubedit DJ/MACWOLF.EXE dpmi=CWSDPR0.EXE

del a_mv_mix.o

goto end

:error
@echo Set the environment variables before running this script!

:end
