/* gradient.h  -- mathematical routines for drawing a gradient
 * $Id: gradient.h,v 1.2 2001/11/11 09:30:01 bitman Exp $
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

#ifndef __GRADIENT_H
#define __GRADIENT_H 1

/* Types of gradients */
#define GRAD_LINEAR   0
#define GRAD_BILINEAR 1
#define GRAD_RADIAL   2
#define GRAD_SCALEDRADIAL   3

typedef struct gradline {
	int x1, y1;
	int x2, y2;
	int type;
	int randomness;
} gradline;

float gradientdistance(gradline grad, int x, int y);
int gradientscaledistance(gradline grad, int x, int y, int length);

#endif
