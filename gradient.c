/* gradient.c  -- mathematical routines for drawing a gradient
 * $Id: gradient.c,v 1.1 2001/11/11 01:17:53 bitman Exp $
 * Copyright (C) 2000 Ryan Phillips <bitman@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "gradient.h"
#include "kevedit.h"

#include <stdlib.h>
#include <math.h>

#define gROUND(n) (n-floor(n)<0.5?floor(n):floor(n)+1)
#define distanceformula(x1, y1, x2, y2) (sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2)))

/* gradientdistance() - returns the distance of point (x, y)'s position on the
 *                      gradient line from the initial gradient point */
float gradientdistance(gradline grad, int x, int y)
{
	float slope, tangentslope;
	float posx, posy;

	/* Distances between points */
	float dg1topos;
	float dg2topos;
	float dg1tog2;

	if (grad.type == GRAD_RADIAL)
		return distanceformula(grad.x1, grad.y1, x, y);

	/* Special cases: slopes of zero and infinity */
	if (grad.y1 == grad.y2) { /* zero slope */
		posx = x;
		posy = grad.y1;
	} else if (grad.x1 == grad.x2) {
		posy = y;
		posx = grad.x1;
	} else {
		slope = (float)(grad.y1 - grad.y2) / (float)(grad.x1 - grad.x2);
		tangentslope = -(float)(grad.x1 - grad.x2) / (float)(grad.y1 - grad.y2);

		posx = ((y - x * tangentslope) - (grad.y1 - slope*grad.x1)) /
			(slope - tangentslope);
		posy = slope * (posx - grad.x1) + grad.y1;

	}
	
	/* Calculate distances */
	dg1topos = distanceformula(posx, posy, grad.x1, grad.y1);
	dg2topos = distanceformula(posx, posy, grad.x2, grad.y2);
	dg1tog2  = distanceformula(grad.x1, grad.y1, grad.x2, grad.y2);

	/* Unless we're doing a bilinear, only allow positions between g1 and g2 */
	if (grad.type != GRAD_BILINEAR) {
		/* If the distance between g1 and pos is less than the distance between
		 * g2 and pos and that distance is greater than the gradient length, then
		 * pos is before point one */
		if (dg1topos < dg2topos && dg2topos > dg1tog2)
			return 0;
		/* In this case, pos is after g2 */
		if (dg1topos > dg2topos && dg1topos > dg1tog2)
			return dg1tog2;
	}

	/* Return the distance from the starting point */
	return dg1topos;
}

int gradientscaledistance(gradline grad, int x, int y, int length)
{
	int result;
	float gradlength, radius;

	gradlength = distanceformula(grad.x1, grad.y1, grad.x2, grad.y2);
	radius = gradientdistance(grad, x, y);

	result = (int)gROUND(length * (radius / gradlength));

	if (result > length)
		return length;
	else
		return result;
}

#if 0
/* temporary */
#include <stdio.h>
#include <conio.h>
#include <dos.h>

char lookuptable[5] = { '\xDB', '\xB2', '\xB1', '\xB0', ' ' };

void drawradial(gradline grad, int width, int height)
{
	int x, y;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			int pos = gradientscaledistance(grad, x, y, 4, 0);
			if (pos < 0)
				pos = 0;
			if (pos > 25)
				pos = 25;
			if (x == grad.x1 && y == grad.y1)
				putch('*');
			else if (x == grad.x2 && y == grad.y2)
				putch('.');
			else
				putch(lookuptable[pos]);
		}
		printf("\n");
	}
}

#define DELAY 200

int main()
{
	int i;
	gradline grad = { 9,1, 1,1 };

	for (i = 0; i < 10; i++) {
		clrscr();
		drawradial(grad, 20, 15);
		delay(DELAY);
		grad.y1++;
	}

	for (i = 0; i < 15; i++) {
		clrscr();
		drawradial(grad, 20, 15);
		delay(DELAY);
		grad.x2++;
	}

	for (i = 0; i < 13; i++) {
		clrscr();
		drawradial(grad, 20, 15);
		delay(DELAY);
		grad.y2++;
	}

	for (i = 0; i < 13; i++) {
		clrscr();
		drawradial(grad, 20, 15);
		delay(DELAY);
		grad.x2--;
	}

	return 0;
}

/* entemporary */
#endif
