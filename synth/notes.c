/* notes.c	-- Generate musical notes in chromatic scale
 * $Id: notes.c,v 1.2 2002/04/04 21:13:25 kvance Exp $
 * Copyright (C) 2001 Kev Vance <kev@kvance.com>
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

#include <math.h>

#include "notes.h"

/* Return the frequency of the given note, "octave" octaves from middle */
float NoteFreq(int note, int octave)
{
	float retval;
	float fraction;

	/* Multiply or divide the base frequency to get to the correct octave
	   relative to A */
	if(octave > 0)
		retval = (float)BASE_PITCH*pow(2, octave);
	else
		retval = (float)BASE_PITCH/pow(2, octave*(-1));
	if(note < NOTE_A)
		retval /= 2;

	/* Find the size of a half step */
	fraction = (log(retval*2) - log(retval))/12;

	/* Move base freq to log */
	retval = log(retval);

	/* Add half-steps to reach the desired note)
	   desired note */
	if(note < NOTE_A)
		retval += (note*(-1))*fraction;
	else if(note > NOTE_A)
		retval += note*fraction;

	/* Get out of log */
	retval = exp(retval);

	/* Return the frequency */
	return retval;
}


