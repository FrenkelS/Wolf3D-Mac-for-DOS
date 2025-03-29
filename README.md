## MacWolf for DOS
![MacWolf for DOS](readme_imgs/macwolf.gif?raw=true)

The Macintosh version of Wolfenstein 3D has different graphics, sound, music and levels compared to the DOS version.
This is a port of the Macintosh version to DOS.
There are two versions: a 32-bit version and a 16-bit version.

The 32-bit version requires a 386 CPU and supports PC Speaker sound effects, and digital sound effects through a Sound Blaster.
Music is supported via Adlib, Sound Blaster, Pro Audio Spectrum, General MIDI, Wave Blaster and Sound Canvas.

The 16-bit version requires a 286 CPU and 2084 kB of XMS memory.
It only supports PC Speaker sound effects and no music.
64 kB of EMS memory is used, if available, as an upper memory block to store data.

Both versions require a VGA graphics card.

It's based on the original [Macintosh Wolfenstein 3D source release](https://github.com/Blzut3/Wolf3D-Mac).

Download MacWolf for DOS [here](https://github.com/FrenkelS/Wolf3D-Mac-for-DOS/releases).

Watch what it looks like on a real PC [here](https://www.youtube.com/watch?v=69g_g_kB9bQ).

**What's special?:**
 - Supports only Wolfenstein 3D: First Encounter, that is, the first 3 levels
 - The palette changes when the player gets hurt or picks up an item 
 - Screen resizing
 - High, low and potato detail modes
 - PC speaker sound effects
 - No saving and loading
 - No joystick support

## Supported video modes

### VGA 320x200 256 color Mode Y
![MacWolf for DOS in 256 colors](readme_imgs/macwolf.png?raw=true)

## Controls:
|Action               |Keys                   |
|---------------------|-----------------------|
|Fire                 |Ctrl & any mouse button|
|Use                  |Enter & Space & E      |
|Sprint               |Shift                  |
|Walk                 |Up & Down & W & S      |
|Turn                 |Left & Right & mouse   |
|Strafe               |Alt                    |
|Strafe left and right|< & > & A & D          |
|Automap              |Tab                    |
|Weapon up            |]                      |
|Switch detail mode   |F5                     |
|Resize screen        |+ & -                  |
|Quit to DOS          |Esc & F10              |

## Command line arguments:
|Command line argument|Effect                              |
|---------------------|------------------------------------|
|baby                 |Difficulty: Can I play daddy?       |
|easy                 |Difficulty: Don't hurt me           |
|normal               |Difficulty: Bring 'em on!           |
|hard                 |Difficulty: I am DEATH incarnate!   |
|nosound              |Disable sound effects and music     |
|nomusic              |Disable music                       |
|mpu401 ###           |Enable MPU-401 at port ###, e.g. 330|
|noal                 |Disable Adlib                       |
|nosfx                |Disable sound effects               |
|nosb                 |Disable Sound Blaster               |
|noems                |Disable EMS                         |
|noxms                |Disable XMS                         |
|nomouse              |Disable mouse                       |

## Cheats:
|Code       |Effects                  |
|-----------|-------------------------|
|XUSCNIELPPA|God mode & Weapons & Keys|
|IDDQD      |God mode                 |
|BURGER     |Weapons                  |
|WOWZERS    |999 bullets              |
|LEDOUX     |God mode & Weapons & Keys|
|SEGER      |Keys                     |
|MCCALL     |Exit Level               |
|APPLEIIGS  |Show pushwalls on automap|
|RATE       |Toggle FPS counter       |

## Easter egg:
Press B at the id Software screen.

## Building 32-bit:
1) Install [Watcom](https://github.com/open-watcom/open-watcom-v2).

2) Run `setenvwc.bat` to set the Watcom environment variables.

3) Run `bw32my.bat` to build `MWOLF386.EXE`

## Building 16-bit:
1) Install [gcc-ia16](https://launchpad.net/%7Etkchia/+archive/ubuntu/build-ia16) (including [libi86](https://gitlab.com/tkchia/libi86)) and [NASM](https://www.nasm.us) on Ubuntu.

2) Run `bg16my.sh` to build `MWOLF286.EXE`

## Building WAD file:
MacWolf for DOS needs a WAD file that's created when `mvn verify` is run in the `wad` folder.
