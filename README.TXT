MacWolf for DOS
Macintosh Wolfenstein 3D ported to DOS
https://github.com/FrenkelS/Wolf3D-Mac-for-DOS

The Macintosh version of Wolfenstein 3D has different graphics, sound, music
and levels compared to the DOS version.
This is a port of the Macintosh version to DOS.
There are two versions: a 32-bit version and a 16-bit version.

The 32-bit version requires a 386 CPU and supports PC Speaker sound effects,
and digital sound effects through a Sound Blaster.
Music is supported via Adlib, Sound Blaster, Pro Audio Spectrum,
General MIDI, Wave Blaster and Sound Canvas.

The 16-bit version requires a 286 CPU and 2084 kB of XMS memory.
It only supports PC Speaker sound effects and no music.
64 kB of EMS memory is used, if available, as an upper memory block to store
data.

Both versions require a VGA graphics card.

It's based on the original Macintosh Wolfenstein 3D source release.

What's special?
 - Supports only Wolfenstein 3D: First Encounter, that is, the first 3 levels
 - The palette changes when the player gets hurt or picks up an item 
 - Screen resizing
 - High, low and potato detail modes
 - PC speaker sound effects
 - No saving and loading
 - No joystick support

Supported video modes:
|Video mode                    |Executable 286|Executable 386|
|------------------------------|--------------|--------------|
|VGA 320x200 256 color Mode Y  |MWOLF286.EXE  |MWOLF386.EXE  |

Controls:
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

Cheats:
|Code      |Effects                  |
|----------|-------------------------|
|XUSCNIELPPA|God mode & Weapons & Keys|
|IDDQD      |God mode                 |
|BURGER     |Weapons                  |
|WOWZERS    |999 bullets              |
|LEDOUX     |God mode & Weapons & Keys|
|SEGER      |Keys                     |
|MCCALL     |Exit Level               |
|APPLEIIGS  |Show pushwalls on automap|
|RATE       |Toggle FPS counter       |

Command line arguments:
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
