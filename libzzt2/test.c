#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "zzt.h"

void printblock(ZZTblock * b) {
	int x, y;
	for (y = 0; y < b->height; y++) {
		for (x = 0; x < b->width; x++) {
			char c = zztTileGetDisplayChar(b, x, y);
			if (c != '\n')
				printf("%c", c);
			else
				printf(" ");
		}
		printf("\n");
	}
}

void printboard(ZZTworld * w) {
	printblock(zztBoardGetCurPtr(w)->bigboard);
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
	ZZTblock *area;

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

	/* block copy testing */
	area = zztBlockCopyArea(zztBoardGetCurPtr(w)->bigboard, 11, 0, 52, 8);
	printblock(area);
	zztBlockPaste(zztBoardGetCurPtr(w)->bigboard, area, 11, 8);

	/* Print the current board */
	printboard(w);

#if 0
	/* Shuffle things up a bit */
	shuffle(w);

	/* Rename to SHUFFLE */
	zztWorldSetFilename(w, "shuffle.zzt");
	zztWorldSetTitle(w, "SHUFFLE");

	/* Unlock */
	zztWorldSetFlag(w, 0, "");

	/* Save */
	if(!zztWorldSave(w))
		printf("Error saving world!\n");
	else
		printf("Saved test\n");
#endif

	/* Clean up */
	zztWorldFree(w);
	zztBlockFree(area);

	return 0;
}

