#include <stdlib.h>
#include <time.h>

#include "zzt.h"

int main()
{
	ZZTworld *w;
	int i, src, dest;
	float bcount;

	/* Load TOWN */
       	w = zztWorldLoad("town.zzt");
	if(w == NULL) {
		printf("Error loading world!\n");
		exit(1);
	}
	printf("Loaded town\n");

	/* Rename to TEST */
	zztWorldSetFilename(w, "shuffle.zzt");
	zztWorldSetTitle(w, "SHUFFLE");
	/* Unlock */
	zztWorldSetFlag(w, 0, "");
	/* Initialize random number generator */
	srand(time(NULL));
	/* Swap random boards for a while */
	bcount = zztWorldGetBoardcount(w) - 1.0;
	for(i = 0; i < 10000; i++) {
		src = (int) 1 + (bcount*rand()/(RAND_MAX+1.0));
		dest = (int) 1 + (bcount*rand()/(RAND_MAX+1.0));
		printf("%i -> %i\n", src, dest);
		zztWorldMoveBoard(w, src, dest);
	}

	/* Save */
	if(!zztWorldSave(w))
		printf("Error saving world!\n");
	else
		printf("Saved test\n");

	/* Clean up */
	zztWorldFree(w);

	return 0;
}
