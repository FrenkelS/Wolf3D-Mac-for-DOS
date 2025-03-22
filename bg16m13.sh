mkdir GCCIA16

unset CFLAGS

export RENDER_OPTIONS=""

export CPU=$1
export OUTPUT=$2

if [ -z "$CPU" ]
then
  #export CPU=i8088
  export CPU=i286
fi

if [ -z "$OUTPUT" ]
then
  export OUTPUT=MACWOLF.EXE
fi

nasm z_xms.asm -f elf -DCPU=$CPU

ia16-elf-gcc -c Data.c      $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -Os -fomit-frame-pointer #-flto -fwhole-program

ia16-elf-gcc -c InterMis.c  $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -Os -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c Intro.c     $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -Os -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c Level.c     $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -Os -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c Music.c     $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -Os -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c Refresh2.c  $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -Os -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c SnesMain.c  $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -Os -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c StateDef.c  $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -Os -fomit-frame-pointer -flto -fwhole-program

ia16-elf-gcc -c a_taskmn.c  $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer #-flto -fwhole-program

ia16-elf-gcc -c a_pcfx.c    $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c Doors.c     $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c EnMove.c    $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c EnThink.c   $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c i_aud16.c   $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c i_input.c   $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c i_main.c    $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c i_pal256.c  $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c i_timer.c   $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c i_vm13.c    $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c Missiles.c  $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c PlMove.c    $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c PlStuff.c   $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c PlThink.c   $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c PushWall.c  $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c RefBsp.c    $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c Refresh.c   $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c RefSprite.c $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c Sight.c     $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c w_wad.c     $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c WolfIO.c    $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c WolfMain.c  $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program
ia16-elf-gcc -c z_zone.c    $RENDER_OPTIONS -march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer -flto -fwhole-program

export CFLAGS="-march=$CPU -mcmodel=small -mnewlib-nano-stdio -mregparmcall -O1 -fomit-frame-pointer"
#export CFLAGS="$CFLAGS -Wall -Wextra"

export GLOBOBJS="  a_pcfx.o"
export GLOBOBJS+=" a_taskmn.o"
export GLOBOBJS+=" Data.o"
export GLOBOBJS+=" Doors.o"
export GLOBOBJS+=" EnMove.o"
export GLOBOBJS+=" EnThink.o"
export GLOBOBJS+=" i_aud16.o"
export GLOBOBJS+=" i_input.o"
export GLOBOBJS+=" i_main.o"
export GLOBOBJS+=" i_pal256.o"
export GLOBOBJS+=" i_timer.o"
export GLOBOBJS+=" i_vm13.o"
export GLOBOBJS+=" InterMis.o"
export GLOBOBJS+=" Intro.o"
export GLOBOBJS+=" Level.o"
export GLOBOBJS+=" Missiles.o"
export GLOBOBJS+=" Music.o"
export GLOBOBJS+=" PlMove.o"
export GLOBOBJS+=" PlStuff.o"
export GLOBOBJS+=" PlThink.o"
export GLOBOBJS+=" PushWall.o"
export GLOBOBJS+=" RefBsp.o"
export GLOBOBJS+=" Refresh.o"
export GLOBOBJS+=" Refresh2.o"
export GLOBOBJS+=" RefSprite.o"
export GLOBOBJS+=" Sight.o"
export GLOBOBJS+=" SnesMain.o"
export GLOBOBJS+=" StateDef.o"
export GLOBOBJS+=" w_wad.o"
export GLOBOBJS+=" WolfIO.o"
export GLOBOBJS+=" WolfMain.o"
export GLOBOBJS+=" z_xms.o"
export GLOBOBJS+=" z_zone.o"

ia16-elf-gcc $GLOBOBJS $CFLAGS $RENDER_OPTIONS -li86 -o GCCIA16/$OUTPUT

rm z_xms.o

rm a_pcfx.o
rm a_taskmn.o
rm Data.o
rm Doors.o
rm EnMove.o
rm EnThink.o
rm i_aud16.o
rm i_input.o
rm i_main.o
rm i_pal256.o
rm i_timer.o
rm i_vm13.o
rm InterMis.o
rm Intro.o
rm Level.o
rm Missiles.o
rm Music.o
rm PlMove.o
rm PlStuff.o
rm PlThink.o
rm PushWall.o
rm RefBsp.o
rm Refresh.o
rm Refresh2.o
rm RefSprite.o
rm Sight.o
rm SnesMain.o
rm StateDef.o
rm w_wad.o
rm WolfIO.o
rm WolfMain.o
rm z_zone.o
