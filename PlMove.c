#include "wolfdef.h"

/**********************************

	See if players current position is ok
	returns True if move ok
	All coordinates are stored in 8.8 fractions (8 bits integer, 8 bits fraction)
		
**********************************/

static Boolean TryMove(Word Checkx,Word Checky)
{
	actor_t *ActorPtr;	/* Pointer to actor record */
	Word xl,yl,xh,yh;	/* Rect to scan */
	Word tile;	/* Tile being tested */
	Word x,y;	/* Working x,y */

	xl = (Checkx - PLAYERSIZE)>>FRACBITS;	/* Make the rect for the player in tiles */
	xh = ((Checkx + PLAYERSIZE)>>FRACBITS)+1;
	yl = (Checky - PLAYERSIZE)>>FRACBITS;
	yh = ((Checky + PLAYERSIZE)>>FRACBITS)+1;
	
/* check for solid walls*/

	y = yl;
	do {
		x=xl;
		do {
			tile = tilemap[y][x];	/* Test the tile */
			if (tile & TI_GETABLE) {	/* Can I get this? */
				GetBonus(x,y);		/* Get the item */
			}
			if (tile & TI_BLOCKMOVE) {	/* Motion blocked */
				return FALSE;		/* Can't go this way! */
			}
		} while (++x<xh);	/* Scan the x */
	} while (++y<yh);		/* Scan the y */

/* check for actors */

	yl--;	/* Increase the rect for actor hits */
	yh++;
	xl--;
	xh++;
	y = yl;
	do {
		x = xl;
		do {	
			tile = tilemap[y][x];	/* Get the tile */
			if (tile&TI_ACTOR) {	/* Actor here? */
				ActorPtr = &actors[MapPtr->tilemap[y][x]];
				if (w_abs(ActorPtr->x - Checkx) < MINACTORDIST) {
					if (w_abs(ActorPtr->y - Checky) < MINACTORDIST)
						return FALSE;		/* I hit the actor! */
				}
			}
		} while (++x<xh);		/* Scan the x */
	} while (++y<yh);			/* Scan the y */
	playermoving = TRUE;		/* I am moving (Harder to hit) */
	return TRUE;				/* Way is clear */
}

/**********************************

	Clip the player's motion
	I will also try to use as much motion as I have
		
**********************************/

static void ClipMove(int xmove,int ymove)
{
	int Checkx,Checky;

/* Try complete move */

	Checkx = xmove;		/* Save the current motion offset */
	Checky = ymove;
	do {
		if (TryMove(actors[0].x+Checkx,actors[0].y+Checky)) {	/* Can I move here? */
			actors[0].x += Checkx;	/* Save it */
			actors[0].y += Checky;
			if (Checkx==xmove) {	/* Use all the motion? */
				return;				/* Exit now! */
			}
			xmove-=Checkx;	/* Remove the used motion */
			ymove-=Checky;
			break;			/* Exit now */
		}
		if (Checkx==-1) {	/* Minimum motion */
			Checkx = 0;
		}
		Checkx>>=1;		/* Reduce the amount of motion */
		if (Checky==-1) {	/* Minimum motion */
			Checky=0;
		}
		Checky>>=1;
	} while (Checkx || Checky);	/* Is there ANY motion? */
	
/* Try horizontal motion */

	Checkx = actors[0].x + xmove;
	if (TryMove(Checkx,actors[0].y)) {
		actors[0].x = Checkx;	/* Set new x */
		return;
	}

/* try just vertical*/

	Checky = actors[0].y + ymove;
	if (TryMove(actors[0].x,Checky)) {
		actors[0].y = Checky;	/* Set new y */
	}
}

/**********************************

	Adds movement to xmove & ymove
		
**********************************/

static void Thrust(Word angle,Word speed,Word *xmove, Word *ymove)
{	
	angle &= ANGLES-1;		/* Mask the angle range */
	if (speed >= TILEGLOBAL) {
		speed = TILEGLOBAL-1;	/* Force maximum speed */
	}
	*xmove += FixedByFrac(speed,costable[angle]);	/* Add x motion */
	*ymove -= FixedByFrac(speed,sintable[angle]);	/* Add y motion */
}

/**********************************

	Changes the player's angle and position
		
**********************************/

#define TURNSPEED		4		/* Turning while moving */
#define FASTTURN		6		/* Turning in place */
#define WALKSPEED		25
#define RUNSPEED		45

void ControlMovement(void)
{
	Word turn,total;
	Word tile;
	Word xmove,ymove;
	Word move;
	
	playermoving = FALSE;	/* No motion (Yet) */
	xmove = 0;				/* Init my deltas */
	ymove = 0;
	
	if (buttonstate[bt_run]) {
		turn = FASTTURN*TicCount;	/* Really fast turn */
		move = RUNSPEED*TicCount;
	} else {
		turn = TURNSPEED*TicCount;	/* Moderate speed turn */
		move = WALKSPEED*TicCount;
	}

/* turning */

	gamestate.viewangle += mouseturn;		/* Add in the mouse movement verbatim */
	
	if (buttonstate[bt_east]) {
		gamestate.viewangle -= turn;		/* Turn right */
	}
	if (buttonstate[bt_west]) {
		gamestate.viewangle += turn;		/* Turn left */
	}
	gamestate.viewangle &= (ANGLES-1);		/* Fix the viewing angle */

/* Handle all strafe motion */

	if (buttonstate[bt_right]) {	/* Slide right keyboard */
		Thrust(gamestate.viewangle - ANGLES/4, move>>1,&xmove,&ymove);
	}
	if (buttonstate[bt_left]) {		/* Slide left keyboard */
		Thrust(gamestate.viewangle + ANGLES/4, move>>1,&xmove,&ymove);
	}	
	
/* Handle all forward motion */

	total = buttonstate[bt_north] ? move : 0;	/* Move ahead? */
	if (total) {
		Thrust(gamestate.viewangle, total,&xmove,&ymove);	/* Move ahead */
	}
	
	total = buttonstate[bt_south] ? move : 0;	/* Reverse direction */
	if (total) {
		Thrust(gamestate.viewangle+ANGLES/2, total,&xmove,&ymove);
	}
	mouseturn = 0;
	
	if (xmove || ymove) {	/* Any motion? */
		ClipMove(xmove,ymove);	/* Move ahead (Clipped) */
		tile = tilemap[actors[0].y>>FRACBITS][actors[0].x>>FRACBITS];
		if (!(tile&TI_DOOR) ) {		/* Normal tile? */
			actors[0].areanumber = tile & TI_NUMMASK;	/* Set my new area */
		}
	}
}

