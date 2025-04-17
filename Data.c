#include "wolfdef.h"

/**********************************

	Global data used by Wolfenstein 3-D
	
**********************************/

Word tilemap[MAPSIZE][MAPSIZE];		/* Main tile map */
Word ConnectCount;					/* Number of valid interconnects */
Boolean	areabyplayer[MAXAREAS];		/* Which areas can I see into? */
Word numstatics;					/* Number of active static objects */
static_t statics[MAXSTATICS];		/* Data for the static items */
Word numdoors;						/* Number of active door objects */
door_t doors[MAXDOORS];				/* Data for the door items */
Word nummissiles;					/* Number of active missiles */
missile_t missiles[MAXMISSILES];	/* Data for the missile items */
Word numactors;						/* Number of active actors */
actor_t actors[MAXACTORS];			/* Data for the actors */
Word difficulty;					/* 0 = easy, 1= normal, 2=hard*/
gametype_t gamestate;				/* Status of the game (Save game) */
exit_t playstate;					/* Current status of the game */
Word killx,killy;					/* X,Y of the thing that killed you! */
Boolean madenoise;					/* True when shooting or screaming*/
Boolean playermoving;				/* Is the player in motion? */
Boolean	buttonstate[NUMBUTTONS];	/* Current input */
Word joystick1;						/* Joystick value */
Boolean strafe;
Boolean mousebutton;
int	mouseturn;						/* Mouse turn movement */
Word nextmap;						/* Next map to warp to */
Word facecount;						/* Time to show a specific head */
Word faceframe;						/* Head pic to show */
Boolean firstframe;					/* if TRUE, the screen is still faded out */
loadmap_t __far* MapPtr;					/* Pointer to current loaded map */
int clipshortangle;					/* Angle for the left edge of the screen */
int clipshortangle2;				/* clipshortangle * 2 */
Word viewx;							/* X coord of camera */
Word viewy;							/* Y coord of camera */
Word normalangle;					/* Normalized angle for view (NSEW) */
Word centerangle;					/* viewangle in fineangles*/
Word centershort;					/* viewangle in 64k angles*/
Word topspritescale;				/* Scale of topmost sprite */
Word topspritenum;					/* Shape of topmost sprite */
Word xscale[1024];	/* Scale factor for width of the screen */
Word numvisspr;				/* Number of valid visible sprites */
vissprite_t	vissprites[MAXVISSPRITES];	/* Buffer for sprite records */
Word *firstevent;			/* First event in sorted list */
Boolean areavis[MAXAREAS];	/* Area visible */
Word bspcoord[4];			/* Rect for the BSP search */
Word TicCount;				/* Ticks since last screen draw */
Boolean IntermissionHack;		/* Hack for preventing double score drawing during intermission */

Word ArtData[64];
Word SpriteArray[S_LASTONE];
Boolean NoWeaponDraw=TRUE;			/* Flag to not draw the weapon on the screen */
maplist_t __far* MapListPtr;		/* Pointer to map info record */
unsigned short __far* SongListPtr;	/* Pointer to song list record */
unsigned short __far* WallListPtr;	/* Pointer to wall list record */
Boolean ShowPush;			/* Cheat for pushwalls */
Boolean ShowFps;
Word viewwidth       = SCREENWIDTH;
Word scaledviewwidth = SCREENWIDTH;
Word viewheight      = MAXVIEWHEIGHT;
Word detailshift     = 0;
Word automapzoomlevel = 4;
Byte textures[MAPSIZE*2+5][MAPSIZE];	/* Texture indexes */
const Word NaziSound[] = {SND_ESEE,SND_ESEE2,SND_ESEE3,SND_ESEE4};

const classinfo_t	classinfo[] = {	/* Info for all the bad guys */
	{SND_ESEE,SND_EDIE,		/* Nazi */
	ST_GRD_WLK1, ST_GRD_STND, ST_GRD_ATK1,ST_GRD_PAIN,ST_GRD_DIE,
	100, 5, 0x0F, 6},
	
	{SND_ESEE,SND_EDIE,	/* Blue guard */
	ST_OFC_WLK1, ST_OFC_STND, ST_OFC_ATK1,ST_OFC_PAIN,ST_OFC_DIE,
	400, 10, 0x01, 12},
	
	{SND_ESEE,SND_EDIE,	/* White officer */
	ST_SS_WLK1, ST_SS_STND, ST_SS_ATK1,ST_SS_PAIN,ST_SS_DIE,
	500, 6, 0x07, 25},
	
	{SND_DOGBARK,SND_DOGDIE,	/* Dog */
	ST_DOG_WLK1,ST_DOG_STND,ST_DOG_ATK1,ST_DOG_WLK1,ST_DOG_DIE,
	200, 9, 0x07, 1},
	
	{SND_NOSOUND,SND_EDIE,		/* Mutant */
	ST_MUTANT_WLK1, ST_MUTANT_STND, ST_MUTANT_ATK1,ST_MUTANT_PAIN,ST_MUTANT_DIE,
	400, 7, 0x01, 18},
	
	{SND_GUTEN,SND_EDIE,			/* Hans */
	ST_HANS_WLK1, ST_HANS_STND, ST_HANS_ATK1,ST_GRD_STND,ST_HANS_DIE,
	5000,7, 0x01, 250},
	
	{SND_SHITHEAD,SND_EDIE,			/* Dr. Schabbs */
	ST_SCHABBS_WLK1, ST_SCHABBS_STND, ST_SCHABBS_ATK1,ST_GRD_STND,ST_SCHABBS_DIE,
	5000, 5,0x01, 350},
	
	{SND_GUTEN,SND_EDIE,			/* Trans */
	ST_TRANS_WLK1, ST_TRANS_STND, ST_TRANS_ATK1,ST_GRD_STND,ST_TRANS_DIE,
	5000, 7,0x01, 300},
	
	{SND_DOGBARK,SND_EDIE,		/* Uber knight */
	ST_UBER_WLK1, ST_UBER_STND, ST_UBER_ATK1,ST_GRD_STND,ST_UBER_DIE,
	5000, 8,0x01, 400},
	
	{SND_COMEHERE,SND_EDIE,			/* Dark knight */
	ST_DKNIGHT_WLK1, ST_DKNIGHT_STND, ST_DKNIGHT_ATK1,ST_GRD_STND,ST_DKNIGHT_DIE,
	5000, 7,0x01, 450},
	
	{SND_SHIT,SND_EDIE,			/* Mechahitler */
	ST_MHITLER_WLK1, ST_MHITLER_STND, ST_MHITLER_ATK1,ST_GRD_STND, ST_HITLER_DIE,
	5000, 7,0x01, 500},
	
	{SND_HITLERSEE,SND_EDIE,			/* Hitler */
	ST_HITLER_WLK1, ST_MHITLER_STND, ST_HITLER_ATK1,ST_GRD_STND,ST_HITLER_DIE,
	5000, 8,0x01, 500},

	{SND_NOSOUND,SND_NOSOUND,			/* Player */
	ST_GRD_STND, ST_GRD_STND, ST_GRD_STND, ST_GRD_STND, ST_GRD_STND,
	0, 5,0x01, 500},

	{SND_NOSOUND,SND_NOSOUND,			/* Green ghost */
	ST_GREEN_GHOST, ST_GREEN_GHOST, ST_GREEN_GHOST, ST_GREEN_GHOST, ST_GREEN_GHOST,
	0, 5,0x01, 500},

	{SND_NOSOUND,SND_NOSOUND,			/* Blue ghost */
	ST_BLUE_GHOST, ST_BLUE_GHOST, ST_BLUE_GHOST, ST_BLUE_GHOST, ST_BLUE_GHOST,
	0, 5,0x01, 500},

	{SND_NOSOUND,SND_NOSOUND,			/* Yellow ghost */
	ST_YELLOW_GHOST, ST_YELLOW_GHOST, ST_YELLOW_GHOST, ST_YELLOW_GHOST, ST_YELLOW_GHOST,
	0, 5,0x01, 500},

	{SND_NOSOUND,SND_NOSOUND,			/* Red ghost */
	ST_RED_GHOST, ST_RED_GHOST, ST_RED_GHOST, ST_RED_GHOST, ST_RED_GHOST,
	0, 5,0x01, 500}
};
