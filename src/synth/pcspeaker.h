/* pcspeaker.c	-- PC speaker music synthesizer
 * $Id: pcspeaker.h,v 1.2 2005/06/29 03:20:34 kvance Exp $
 * Copyright (C) 2001 Kev Vance <kvance@kvance.com>
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
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#ifndef PC_SPEAKER_H
#define PC_SPEAKER_H 1

#include "notes.h"

/* Play a single note. Program will halt until playback has finished. */
void pcSpeakerPlayNote(musicalNote note, musicSettings settings);

/* Call immediately after note playback to silence the speaker */
void pcSpeakerFinish(void);

#endif
