/* hash.h       -- Rudimentary hash table for bind index merging
 * $Id: hash.h,v 1.3 2023/11/12 18:09:18 kristomu Exp $
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

#ifndef _HASH_H
#define _HASH_H 1

#include "libzzt2/zzt.h"

/* This is a preset size hash table with open hashing. Because a board
 * may have multiple copies of the same object, we need a linked list
 * to hold all of them anyway, so there's no reason to use closed
 * hashing. The table is an array of pointers to linked lists that
 * start out as NULL, and to keep it short, can't be resized. This
 * should pose no problem as we know the number of params in a board
 * beforehand. */

/* The first node's previous pointer points to the last node. */

typedef struct llnode {
	ZZTparam * param;
	struct llnode * previous;
	struct llnode * next;
} llnode;

typedef struct hash_table {
	llnode ** table;
	int size;
} hash_table;

hash_table hashInit(int max_items);
void addNode(hash_table * htab, ZZTparam * param);
void addNodes(hash_table * htab, ZZTblock * block);
void removeNode(hash_table * htab, llnode * node);

/* Returns the head of the list that contains every entry with this
 * hash. It may also contain entries with other hashes if there are
 * collisions. */
const llnode * getFirstByHash(hash_table * htab, ZZTparam * param);

int isProgramEqual(ZZTparam * a, ZZTparam * b);

/* Returns the first node containing a param with the same code as
 * param. NULL if none. */
const llnode * getFirstEqual(hash_table * htab, ZZTparam * param);
const llnode * getNextEqual(const llnode * last, ZZTparam * param);

void freeTable(hash_table * htab);

#endif