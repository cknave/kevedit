#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "zzt.h"

void printboard(ZZTworld * w) {
	int x, y;
	for (y = 0; y < w->boards[zztBoardGetCurrent(w)].bigboard->height; y++) {
		for (x = 0; x < w->boards[zztBoardGetCurrent(w)].bigboard->width; x++) {
			printf("%c", zztGetDisplayChar(w, x, y));
		}
		printf("\n");
	}
}

void shuffle(ZZTworld * w) {
	int i;
	float bcount;
	int src, dest;

	/* Initialize random number generator */
	srand(time(NULL));
	/* Swap random boards for a while */
	bcount = zztWorldGetBoardcount(w) - 1.0;
	for(i = 0; i < 100; i++) {
		src = (int) 1 + (bcount*rand()/(RAND_MAX+1.0));
		dest = (int) 1 + (bcount*rand()/(RAND_MAX+1.0));
		printf("%i -> %i\n", src, dest);
		zztWorldMoveBoard(w, src, dest);
	}
}

int main(int argc, char * argv[])
{
	ZZTworld *w;
	char loadname[1000] = "town.zzt";
	ZZTtile foo = { ZZT_EMPTY, 0x07, NULL };

	if (argc > 1) {
		strcpy(loadname, argv[1]);
	}

	/* Load TOWN */
	w = zztWorldLoad(loadname);
	if(w == NULL) {
		printf("Error loading world!\n");
		exit(1);
	}
	printf("Loaded %s\n", loadname);

	/* Rename to SHUFFLE */
	zztWorldSetFilename(w, "shuffle.zzt");
	zztWorldSetTitle(w, "SHUFFLE");

	/* Unlock */
	zztWorldSetFlag(w, 0, "");

	zztBoardSelect(w, 1);

	/* Copy something */
	foo = zztTileGet(w, 32, 21);
	zztPlot(w, 4, 11, foo);

	/* Move the player */
	zztPlotPlayer(w, 4, 11);

	/* Print the current board */
	printboard(w);

	/* Shuffle things up a bit */
	shuffle(w);

	/* Save */
	if(!zztWorldSave(w))
		printf("Error saving world!\n");
	else
		printf("Saved test\n");

	/* Clean up */
	zztWorldFree(w);

	return 0;
}

