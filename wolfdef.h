#ifndef __WOLFDEF__
#define __WOLFDEF__

#define TICRATE 60

#define DEMO		/* Define if this is the lame demo for dealers */

/* japversion has mission pics instead */
/* #define JAPVERSION */

/* If code is compiled on a IIgs, pass the compiler presets... */

#ifdef __ORCAC__
#pragma optimize 15		/* Normal optimization */
#pragma memorymodel 0	/* Force small memory model */
#pragma noroot			/* No root files */
#pragma lint -1			/* Full error checking */
segment "Wolf3d";		/* Code segment */
#endif

/* an angle_t occupies an entire 16 bits so wraparound is automatically handled */

#define	SHORTTOANGLESHIFT	7	/* 0x10000 to ANGLES */
#define	SLOPEBITS 9		/* Used in AngleFromSlope2 */

typedef unsigned short angle_t;		/* Must be short to allow wraparound */
typedef short fixed_t;				/* 8.8 fixed point number */	
typedef unsigned short ufixed_t;	/* 8.8 unsigned fixed point number */

#include "burger.h"		/* My standard system equates */
#include "i_system.h"
#include "States.h"		/* Think state equates */
#include "Sounds.h"		/* Sound equates */
#include "Sprites.h"	/* Sprite indexs */
#include "Wolf.h"		/* Resource maps */

/**********************************

	Game constants and equates
	
	The game uses a tile map of 64X64 tiles and uses a 16 bit fixed
	point number to record the placement of any object in the game. The
	upper 8 bits are the tile with the lower 8 bits being which fraction into the
	tile the player is standing on. 
	
	To adjust the screen size, modify SCREENWIDTH,SCREENHEIGHT,viewwidth,viewheight,ScaleX,ScaleY
	
**********************************/

#define	MAXVISSPRITES 64	/* Maximum number of sprites to display (Must be a power of 2!) */
#define MAXACTORS	128		/* max number of nazis, etc / map */
#define MAXSTATICS	200		/* max number of lamps, bonus, etc */
#define MAXMISSILES	32		/* max missiles, flames, smokes, etc */
#define MAXDOORS	64		/* max number of sliding doors (64<=) */
#define MAXAREAS	64		/* max number of bsp areas */
#define	NUMBUTTONS	10		/* Number of control buttons */
#define WALLHEIGHT 128		/* Height,width of walls */
#define SPRITEHEIGHT 64		/* Height,width of a sprite */
#define MAXSCALER 960		/* Number of scalers to create */

#define	PLAYERSIZE	88		/* radius of player in 8.8 fixed point */
#define	FIELDOFVIEW	364*4 /* fineangles in the SCREENWIDTH wide window*/
#define MINZ 62			/* PLAYERSIZE/sqrt(2) rounded down*/
#define	MAXZ (32*FRACUNIT)	/* Farthest thing away */
#define	FINEANGLES	0x2000		/* Power of 2 */
#define	FINEMASK	(FINEANGLES-1)	
#define	ANGLETOFINESHIFT	3	/* 0x10000 >> 0x2000*/
#define GAMEANGLETOFINE		4	/* 512 << 0x2000 */
#define	ANGLE90		0x4000		/* Use a 0x10000 angle range */
#define	ANGLE180	0x8000		/* Use a 0x10000 angle range */

#ifdef __MAC__
#define	SCREENWIDTH	MacWidth		/* Size of the offscreen buffer */
#define SCREENHEIGHT MacHeight	/* Height of the offscreen buffer */
Word ScaleX(Word x);		/* Scale factor for 320 mode points projected to SCREEN */
Word ScaleY(Word y);
extern Word MacWidth;
extern Word MacHeight;
extern Word MacViewHeight;
#else
#define	SCREENWIDTH	320		/* Size of the offscreen buffer */
#define SCREENHEIGHT 200	/* Height of the offscreen buffer */
#define MAXVIEWHEIGHT 160
#define ScaleX(x) x		/* Scale factor for 320 mode points projected to SCREEN */
#define ScaleY(y) y
#endif

#define	centerx	(viewwidth/2)	/* Center of the viewing area */

#define	ANGLES		512		/* Number of angles for camera */
#define	FRACBITS	8		/* Number of bits of fraction */
#define	FRACUNIT	(1<<FRACBITS)	/* Shift count for fraction conversion */
#define	MAXFRAC	0x7fff		/* Largest fraction constant */
#define	MAXUFRAC	0xffff	/* Largest unsigned fraction constant */
#define MAXDAMAGECOLOR 32	/* Number of shades of red to use for damage */
#define MAXBONUSCOLOR 8		/* Number of shades of gold to use for damage */

#define EXTRAPOINTS	20000	/* Points for a free man */
#define STARTAMMO	16		/* Ammo to begin the game with */
#define TILEGLOBAL 	 256	/* Pixels per tile */
#define MINACTORDIST 192	/* minimum dist from player center*/
							/* to any actor center*/
#define KNIFEDIST	480		/* max dist for a knife hit (fixed_t) */
#define BITERANGE	350		/* max dist for a bite hit (fixed_t) */
#define MISSILEHITDIST 120	/* max dist for a missile impact (fixed_t) */
#define MAPSIZE		64		/* Size of a map in tiles */


/* joypad masks */

#define JOYPAD_B	0x8000
#define JOYPAD_Y	0x4000
#define JOYPAD_SELECT 0x2000
#define JOYPAD_START 0x1000
#define JOYPAD_UP	0x800
#define JOYPAD_DN	0x400
#define JOYPAD_LFT	0x200
#define JOYPAD_RGT	0x100
#define JOYPAD_A	0x80
#define JOYPAD_X	0x40
#define JOYPAD_TL	0x20
#define JOYPAD_TR	0x10

/**********************************

	Game enums and record lists 
	
**********************************/

typedef enum {		/* Think logic states (MUST match thinkcalls in EnThink.c) */
	T_NULL,			/* No action */
	T_STAND,		/* Watch for player */
	T_CHASE			/* Chase player */
} thinklogic_t;
	
typedef enum {		/* Action logic states (MUST match actioncalls in EnThink.c */
	A_NULL,			/* No action */
	A_TARGET,		/* Target the player */
	A_SHOOT,		/* Shoot the player */
	A_BITE,			/* Bite the player */
	A_THROW,		/* Throw a syringe */
	A_LAUNCH,		/* Launch a missile */
	A_HITLERMORPH,	/* Remove the mechhitler's armor */
	A_MECHSTEP,		/* Mechahitler step sound */
	A_VICTORY,		/* You win */
	A_SCREAM,		/* Actor screams */
	A_THUD			/* Hit the ground */
} actionlogic_t;

typedef enum {		/* Index to the button array */
	bt_north,		/* North pressed */
	bt_east,		/* Turn East pressed */
	bt_south,		/* South pressed */
	bt_west,		/* Turn West pressed */
	bt_left,		/* Move left */
	bt_right,		/* Move right */
	bt_attack,		/* Fire */
	bt_use,			/* Open door, use switch */
	bt_run,			/* Move faster */
	bt_select		/* Change weapons */
} buttonname_t;

typedef enum {		/* Used by spawning and wall pushing */
	CD_NORTH,		/* Face north */
	CD_EAST,		/* Face east */
	CD_SOUTH,
	CD_WEST
} cardinaldir_t;

typedef enum {		/* Used by actor's motion */
	east,
	northeast,
	north,
	northwest,
	west,
	southwest,
	south,
	southeast,
	nodir
} dirtype;

typedef enum {		/* Weapons used by the player */
	WP_KNIFE,
	WP_PISTOL,
	WP_MACHINEGUN,
	WP_CHAINGUN,
	WP_FLAMETHROWER,
	WP_MISSILE
} weapontype;

typedef	enum {		/* State of the game */
	EX_LIMBO,		/* Currently in limbo */
	EX_STILLPLAYING,
	EX_COMPLETED,
	EX_SECRET,
	EX_WARPED,
	EX_DIED,
	EX_NEWGAME,
	EX_LOADGAME,
	EX_AUTOMAP
} exit_t;

typedef enum {		/* actor class info*/
	CL_GUARD,
	CL_OFFICER,
	CL_SS,
	CL_DOG,
	CL_MUTANT,
	
	CL_HANS,
	CL_SCHABBS,
	CL_TRANS,
	CL_UBER,
	CL_DKNIGHT,
	CL_MECHAHITLER,
	CL_HITLER,
	
	CL_PLAYER
} class_t;

enum {BSPTOP,BSPBOTTOM,BSPLEFT,BSPRIGHT};	/* BSP quadrants */

/**********************************

	Compiled scaler
	
**********************************/

typedef struct {
	unsigned short codeofs[WALLHEIGHT+1];		/* Entry to the code for sprites */
#ifndef __APPLEIIGS__
	Byte FixA1[WALLHEIGHT+1];		/* A1 adjust for the screen */
	Byte Pad[1];			/* Long word align it... */
#endif
	Byte code[1];					/* Scaler code */
} t_compscale;

/**********************************

	Status of the game (Save game record)
	
**********************************/

typedef	struct {
	LongWord score;	/* Current score */
	LongWord nextextra;	/* Points to next free man */
	LongWord globaltime, globaltimetotal;
	LongWord playtime;					/* Time for the current game (In ticks) */
	Word mapon;		/* Current map */
	Word treasure;	/* Treasures picked up */
	Word lives;		/* Lives remaining */
	Word health;	/* Hit points */
	Word ammo;		/* Current bullets */
	Word maxammo;	/* Maximum bullets */
	Word gas;		/* Flame thrower ammo */
	Word missiles;	/* Missile launcher ammo */
	Word keys;		/* Keys obtained */
	Boolean	machinegun,chaingun,missile,flamethrower;
	weapontype weapon,pendingweapon;
	Word attackframe,attackcount;
	Word secretcount,treasurecount,killcount;
	Word secrettotal,treasuretotal,killtotal;
	Word globalsecret, globaltreasure, globalkill;
	Word globalsecrettotal, globaltreasuretotal, globalkilltotal;
	Word viewangle;						/* Angle of camera */
	Boolean godmode;					/* You are invincible */
} gametype_t;

/**********************************

	Map data record (Stored maps)
	
**********************************/

typedef struct {
	Byte tilemap[64][64];
	Byte areasoundnum[64];
	unsigned short numspawn;		/* Must be short */
	unsigned short spawnlistofs;	/* Must be short */
	unsigned short numnodes;		/* Must be short */
	unsigned short nodelistofs;		/* Must be short */
	Byte data[1];		/* nodes, and spawn list */
} loadmap_t;

typedef struct {
	unsigned short NextLevel;		/* Normal warp level */
	unsigned short SecretLevel;		/* Secret level */
	unsigned short ParTime;			/* Time for par */
	unsigned short ScenarioNum;		/* Scenario number */
	unsigned short FloorNum;		/* Floor number */
} MapInfo_t;

typedef struct {
	unsigned short MaxMap;			/* Maximum number of maps */
	unsigned short MapRezNum;		/* Basic resource # */
	MapInfo_t InfoArray[1];	/* Next map to jump to */
} maplist_t;

/**********************************

	Static data for each sprite state
	
**********************************/

typedef struct {
	Word sightsound;	/* Sound at the sight of you */
	Word deathsound;	/* Sound at the death of the actor */
	stateindex_t sightstate;	/* State when you are sighted */
	stateindex_t standstate;	/* State when at rest */
	stateindex_t attackstate;	/* State when attacking you */
	stateindex_t painstate;		/* State when hit */
	stateindex_t deathstate;	/* State when dead */
	Word points;		/* Points for death */
	Word speed;			/* Speed of actor */
	Word reactionmask;	/* reaction time = 1 + w_rnd()&reactionmask*/
	Word hitpoints;		/* Hit points */
} classinfo_t;

enum {di_north, di_east, di_south, di_west};		/* BSP base directions */

/**********************************

	The saved data structures are held in a single list, with segs being differentiated from 
	nodes by the presence of DIR_SEGFLAG in the dir field
	Note... saveseg_t and savenode_t share the same memory
	
**********************************/

#define	DIR_SEGFLAG		0x80	/* Use segment value */
#define	DIR_LASTSEGFLAG	0x40	/* Last segment in list */
#define	DIR_DISABLEDFLAG 0x20	/* Shut down (Pushwall) */
#define	DIR_SEENFLAG	0x10	/* For automapping*/

typedef struct {
	Byte plane;
	Byte dir;
	unsigned short children[2];
} savenode_t;

#ifndef __BIGENDIAN__
typedef struct {
	Byte	plane;		/* in half tiles*/
	Byte	dir;
	Byte	min,max;	/* in half tiles*/
	Byte	texture;
	Byte	area;
} saveseg_t;
#else
typedef struct {
	Byte	plane;			/* in half tiles*/
	Byte	dir;
	Byte	max,min;		/* in half tiles*/
	Byte	area;
	Byte	texture;
} saveseg_t;
#endif

/**********************************

	Static object struct
	(Bullets,food,gold)
	
**********************************/

/* Used by the renderer, must match the header of static_t, actor_t, missile_t */

typedef struct {
	int	x,y;		/* Item's x,y */
	Word sprite;	/* Item's shape */
	Word areanumber;	/* Item's visible area */
} thing_t;

typedef struct {	/* Must match thing_t */
	Word x,y;	/* Item's x,y */
	Word pic;	/* Picture of item */
	Word areanumber;	/* Which room is it in (Rendering) */
} static_t;

/**********************************

	Static door struct
	
**********************************/

typedef	enum {
	DR_OPEN,		/* Door is fully open */
	DR_CLOSED,		/* Door is fully closed */
	DR_OPENING,		/* Door is opening */
	DR_CLOSING,		/* Door is closing */
	DR_WEDGEDOPEN	/* Door is permenantly open */
} dooraction_t;

#define OPENTICS (2*TICRATE)			/* Time to wait before closing a door (In Ticks) */
#define DOORSPEED (TILEGLOBAL/64) /* Time to close a door (32 ticks) */

typedef struct {
	Word tilex;		/* X coord of door */
	Word tiley;		/* Y coord of door */
	Word action;	/* Action code (See above) for door */
	Word position;	/* Pixel position of door (0=Closed) */
	Word info;		/* Texture of the door (Steel,Elevator) */
	Word ticcount;	/* Time delay before automatic closing */
	Word area1;		/* Area # on one side of the door */
	Word area2;		/* Area # on the other side of the door */
} door_t;

typedef struct {
	Boolean Area1;	/* First area */
	Boolean Area2;	/* Second area */
} connect_t;

/**********************************

	Pushwall state struct
	
**********************************/

typedef struct {
	Word pwallcount;					/* Blocks still to move (Distance) */
	Word pwallpos;						/* Amount a pushable wall has been moved in it's tile */
	Word pwallx,pwally;					/* the tile the pushwall edge is in now*/
	Word pwallcheckx,pwallchecky;		/* the tile it will be in next*/
	Word pwalldir;
	int pwallxchange, pwallychange;  	/* adjust coordinates this much*/
} pushwall_t;

/**********************************

	Sprite state struct
	
**********************************/

typedef struct {
	Word shapenum;			/* Shape to display */
	Word tictime;			/* Time before next state */
	thinklogic_t think;		/* Think logic index */
	actionlogic_t action;	/* Action code */
	stateindex_t next;		/* Next state */
} state_t;

/**********************************

	Visible sprite struct (Used to render the sprites)
	
**********************************/

typedef struct {
	Word lumpNum;			/* lump number */
	ufixed_t columnstep;	/* Step factor for width scale */
	int x1,x2;				/* Left x, Right x */
	Word actornum;			/* 0 if a static sprite / missile*/
	Word clipscale;			/* Size of sprite (Scale number) */
} vissprite_t;

/**********************************

	Data struct for thinking actor
	
**********************************/

/* Actor flags */

#define FL_SHOOTABLE	1
#define FL_ACTIVE		2
#define FL_SEEPLAYER	4		/* if True, dodge instead of moving straight forward*/
#define FL_AMBUSH		16
#define FL_WAITDOOR		32		/* if set, ac_dir points at a door*/
#define FL_NOTMOVING	64		/* still centered in dest tile*/
#define FL_DEAD			128		/* if set, don't shoot this anymore (death explosion)*/

typedef struct {	/* Must match thing_t */
	Word x,y;
	Word pic;
	Word areanumber;

	Word ticcount;	/* Time before motion */
	class_t class;	/* Actor's class */
	stateindex_t state;	/* State index */
	Word flags;		/* State flags (See above) */
	Word distance;	/* Distance to travel before change */
	Word dir;		/* 9 directions for motion */
	Word hitpoints;	/* Hit points before death */
	Word speed;		/* Speed of motion */
	Word goalx;		/* tile x,y for movement goal */
	Word goaly;
	Word reacttime;	/* Time to react to the player */
} actor_t;

/**********************************

	Missile struct
	(Rockets, flames)
	
**********************************/

typedef enum {		/* Flame type */
	MI_PMISSILE,		/* Player's missile */
	MI_PFLAME,			/* Player's flame */
	MI_EMISSILE,
	MI_NEEDLE
} mtype_t;

#define	MF_HITPLAYER	1		/* Can it hit the player? */
#define	MF_HITENEMIES	2		/* Can it hit an enemy? */
#define	MF_HITSTATICS	4		/* Can it hit a static item? */

typedef struct {		/* Must match thing_t */
	Word x,y;			/* Position of the missile */
	Word pic;			/* Picture of missile */
	Word areanumber;	/* Area missile is in */
	Word type;			/* Also used as a ticcount for explosions */
	Word flags;			/* Who can I hit? (If zero then explosion) */
	int	xspeed,yspeed;	/* Direction of travel */
} missile_t;

/**********************************

	Flags used by tilemap, NOTE: this allows only 128 unique tiles
	
**********************************/

#define	TI_SECRET		0x8000		/* Secret level switch */
#define	TI_BODY			0x4000		/* Dead body here */
#define	TI_BLOCKSIGHT	0x2000		/* Sight is blocked */
#define	TI_GETABLE		0x1000		/* Getable item here */
#define	TI_ACTOR		0x800		/* Actor here */
#define	TI_DOOR			0x400		/* Door here */
#define	TI_PUSHWALL		0x200		/* Pushwall here */
#define	TI_SWITCH		0x100		/* Exit switch here */
#define	TI_BLOCKMOVE	0x80		/* Block in motion here */
#define	TI_NUMMASK	0x7f			/* Can be an area, door number, or pwall number */

extern Word ArtData[64];
extern Byte textures[MAPSIZE*2+5][MAPSIZE]; /* 0-63 is horizontal, 64-127 is vertical*/
										/* 128 - 132 are doors*/

/* In Mac.c, 3DO.c, AppleIIgs.c */

#ifndef __MAC__
typedef struct {
    short top;
    short left;
    short bottom;
    short right;
} Rect;
#endif

extern void InitTools(void);
extern void BlastScreen(void);
extern void BlastScreen2(Rect *BlastRect);
extern void BlastView(Boolean refreshStatusBar);
extern void DoMacEvents(void);
extern void GoodBye(void);
extern void ReadSystemJoystick(void);
extern void ShowGetPsyched(void);
extern void DrawPsyched(Word Index);
extern void EndGetPsyched(void);
extern Word ChooseGameDiff(void);
extern void ShareWareEnd(void);
extern void FinishLoadGame(void);

/* In StateDef.c */

extern const state_t states[NUMSTATES];	/* Actor states */

/* In Doors.c */

extern void ConnectAreas(void);
extern void OpenDoor(door_t *door);
extern void OperateDoor(Word dooron);
extern void MoveDoors(void);

/* Missiles.c */

extern missile_t *GetNewMissile(void);
extern void MoveMissiles(void);

/* In Level.c */

extern void ReleaseMap(void);
extern void SetupGameLevel(void);

/* In Sight.c */

extern Boolean CheckLine(actor_t *ActorPtr);
extern void FirstSighting(actor_t *ActorPtr);
extern void T_Stand(actor_t *ActorPtr);

/* In Enmove.c */

extern void NewState(actor_t *ActorPtr,stateindex_t state);
extern Boolean TryWalk(actor_t *ActorPtr);
extern void SelectDodgeDir(actor_t *ActorPtr);
extern void SelectChaseDir(actor_t *ActorPtr);
extern void MoveActor(actor_t *ActorPtr,Word move);

/* In EnThink.c */

extern void DamageActor(Word damage,actor_t *ActorPtr);
extern Word CalcDistance(actor_t *ActorPtr);
extern void MoveActors(void);

/* In PlStuff.c */

extern void TakeDamage(Word points,Word x,Word y);
extern void GivePoints(LongWord points);
extern void GiveAmmo(Word ammo);
extern void GiveGas(Word ammo);
extern void GiveMissile(Word ammo);
extern void GiveKey(Word key);
extern void GetBonus(Word x,Word y);

/* In PlThink.c */

extern void MovePlayer(void);

/* In PlMove.c */

extern void ControlMovement(void);

/* In PushWall.c */

extern pushwall_t PushWallRec;	/* Record for the single pushwall in progress */
extern void PushWall(Word x,Word y,Word dir);
extern void MovePWalls(void);

/* In WolfIO.c */

extern void SetNumber(LongWord number,Word x,Word y,Word digits);
extern void IO_CheckInput(void);
extern void IO_ColorScreen(Word bonus, Word damage);
extern void IO_DrawFloor(Word floor);
extern void IO_DrawScore(LongWord score);
extern void IO_DrawLives(Word lives);
extern void IO_DrawHealth(Word health);
extern void IO_DrawAmmo(Word ammo);
extern void IO_DrawTreasure(Word treasure);
extern void IO_DrawKeys(Word keys);
extern void IO_AttackShape(Word shape);
extern void IO_DrawFace(Word face);
extern void IO_DrawStatusBar(void);
extern void IO_ClearViewBuffer(void);
extern void IO_ScaleWallColumn(Word x,Word scale,Word tile,Word column);
extern void IO_DisplayViewBuffer(void);

/* In SetupScalers.c */

extern Boolean BuildCompScale (Word height, void **finalspot,Byte *WorkPtr);
extern void SetupScalers(void);
extern void IO_ScaleMaskedColumn(Word x,Word scale,Word lumpNum,Word column);
extern void DrawSmall(Word x,Word y,Word tile);
extern void MakeSmallFont(void);
extern void KillSmallFont(void);

/* In Intro.c */

extern void Intro(void);

/* In Music.c */

extern void StopSong(void);
extern void StartSong(Word songnum);

/* In WolfMain.c */

extern Word w_abs(int v);
extern Word w_rnd(void);
extern angle_t PointToAngle(fixed_t x,fixed_t y);
extern void NewGame(void);
extern void RedrawStatusBar(void);
extern void GameLoop(void);
extern void StartDamageFlash(Word damage);
extern void StartBonusFlash(void);

/* In RefSprite.c */

extern void DrawTopSprite(void);
extern void DrawSprites(void);

/* In Refresh.c */

extern savenode_t __far* nodes;
extern saveseg_t __far* pwallseg;
extern fixed_t	FixedByFrac(fixed_t a, fixed_t b);
extern fixed_t	SUFixedMul(fixed_t a, ufixed_t b);
extern fixed_t	FixedDiv(fixed_t a, fixed_t b);
extern fixed_t R_TransformX(fixed_t x,fixed_t y);
extern fixed_t R_TransformZ(fixed_t x,fixed_t y);
extern Word ScaleFromGlobalAngle(int visangle,int distance);
extern void DrawAutomap(Word tx,Word ty);
extern void StartupRendering(void);
extern void NewMap(void);
extern void StartPushWall(void);
extern void AdvancePushWall(void);
extern void RenderView(void);

/* In Refresh2.c */

extern Word __far* scaleatzptr;	/* Pointer to the scale table for z projection */
extern short __far* xtoviewangle;	/* Screen x to view angle */
extern short __far* viewangletox;	/* View angle to screen x */
extern const short finetangent[FINEANGLES/2];	/* Fine tangent table */
extern const short finesine[FINEANGLES/2];		/* Fine sine table */
extern const fixed_t sintable[ANGLES];	/* Course sine table */
extern const fixed_t costable[ANGLES];	/* Course cosine table */
extern const Word tantoangle[513];		/* Course tangent to angle table */
extern void GetTableMemory(void);

/* In RefBsp.c */

extern void ClearClipSegs(void);
extern void RenderBSPNode(Word bspnum);

/* In SnesMain.c */

extern void SetupPlayScreen(void);
extern void RunAutoMap(void);
extern void WolfMain(void);

/* In Intermis.c */

extern void Intermission(void);
extern void VictoryIntermission(void);
extern void CharacterCast(void);

/* In Data.c */

extern Word SpriteArray[S_LASTONE];			/* Resource numbers of all the sprites */
extern Word tilemap[MAPSIZE][MAPSIZE];	/* Main tile map */
extern Word ConnectCount;				/* Number of valid interconnects */
extern Boolean areabyplayer[MAXAREAS];	/* Which areas can I see into? */
extern Word numstatics;					/* Number of active static objects */
extern static_t	statics[MAXSTATICS];	/* Data for the static items */
extern Word numdoors;						/* Number of active door objects */
extern door_t doors[MAXDOORS];				/* Data for the door items */
extern Word nummissiles;					/* Number of active missiles */
extern missile_t missiles[MAXMISSILES];		/* Data for the missile items */
extern Word numactors;						/* Number of active actors */
extern actor_t actors[MAXACTORS];			/* Data for the actors */
extern t_compscale *AllScalers[MAXSCALER];	/* Pointers to all the compiled scalers */	
extern Word difficulty;					/* 0 = easy, 1= normal, 2=hard*/
extern gametype_t gamestate;			/* Status of the game (Save game) */
extern exit_t playstate;				/* Current status of the game */
extern Word killx,killy;				/* X,Y of the thing that killed you! */
extern Boolean madenoise;				/* True when shooting or screaming*/
extern Boolean playermoving;			/* Is the player in motion? */
extern Boolean buttonstate[NUMBUTTONS];	/* Current input */
extern Word joystick1;					/* Joystick value */
extern Boolean strafe;
extern Boolean mousebutton;
extern int mouseturn;					/* Mouse turn factor */
extern Word nextmap;					/* Next map to warp to */
extern Word facecount;					/* Time to show a specific head */
extern Word faceframe;					/* Head pic to show */
extern Boolean firstframe;					/* if TRUE, the screen is still faded out */
extern loadmap_t __far* MapPtr;				/* Pointer to current loaded map */
extern int clipshortangle;				/* Angle for the left edge of the screen */
extern int clipshortangle2;				/* clipshortangle * 2 */
extern const classinfo_t classinfo[];			/* Class information for all bad guys */
extern Word viewx;						/* X coord of camera */
extern Word viewy;						/* Y coord of camera */
extern Word normalangle;				/* Normalized angle for view (NSEW) */
extern Word centerangle;				/* viewangle in fineangles*/
extern Word centershort;				/* viewangle in 64k angles*/
extern Word topspritescale;				/* Scale of topmost sprite */
extern Word topspritenum;				/* Shape of topmost sprite */
extern Word xscale[1024];		/* Scale factor for width of the screen */
extern Word numvisspr;				/* Number of valid visible sprites */
extern vissprite_t	vissprites[MAXVISSPRITES];	/* Buffer for sprite records */
extern Word *firstevent;			/* First event in sorted list */
extern Boolean areavis[MAXAREAS];	/* Area visible */
extern Word bspcoord[4];			/* Rect for the BSP search */
extern Word TicCount;				/* Ticks since last screen draw */
extern Boolean IntermissionHack;	/* Hack for preventing double score drawing during intermission */
extern Boolean NoWeaponDraw;		/* Flag to not draw the weapon on the screen */
extern maplist_t __far* MapListPtr;		/* Pointer to map info record */
extern unsigned short __far* SongListPtr;	/* Pointer to song list record */
extern unsigned short __far* WallListPtr;	/* Pointer to wall list record */
extern const Word NaziSound[];		/* Sounds for nazis starting */
extern Boolean ShowPush;		/* Cheat for showing pushwalls on automap */
extern Boolean ShowFps;
extern Word viewwidth;
extern Word scaledviewwidth;
extern Word viewheight;		/* Height of the viewing area */
extern Word detailshift;

#endif /* __WOLFDEF__ */
