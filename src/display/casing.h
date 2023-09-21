/* casing.h    -- Upper/lowercase functions for code page 437.
 * Copyright (C) 2023 Kristofer Munsterhjelm <kristofer@munsterhjelm.no>
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

#ifndef CASING_H
#define CASING_H 1

/* Note that the following tables follow the unicode.c convention that
   uppercase phi is theta, to simulate ø and Ø. */

/* The corresponding uppercase code page 437 character for any CP437 character */
extern const char cp437_uppercase[256];

/* The corresponding lowercase code page 437 character for any CP437 character */
extern const char cp437_uppercase[256];

/* Uppercasing and lowercasing functions */
char tocupper(int c);
char toclower(int c);

#endif