#include "Wolfdef.h"

/**********************************

	Stop the current song from playing

**********************************/

void StopSong(void)
{
	PlaySong(0);
}

/**********************************

	Play a new song
	
**********************************/

void StartSong(Word songnum)
{
	PlaySong(songnum);			/* Stop the previous song (If any) */
}
