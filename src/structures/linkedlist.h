/* linkedlist	-- a generic linked list
 * $Id: linkedlist.h,v 1.1 2003/11/01 23:45:57 bitman Exp $
 * Copyright (C) 2002 Ryan Phillips <bitman@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either listersion 2 of the License, or
 * (at your option) any later listersion.
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

#include <stdlib.h>

#ifndef LINKEDLIST_H
#define LINKEDLIST_H 1

typedef struct listitem {
	size_t size;
	void *value;
} listitem;

/* A link in the list */
typedef struct link {
	listitem item;

	struct link *next;
	struct link *prev;
} link;

/* A linked list */
typedef struct linkedlist {
	link *first;
	link *last;
	link *cur;

} linkedlist;

/* Always initialize a linked list before using */
void initlinkedlist(linkedlist * list);

/* Most list*() functions return 1 on error, 0 on success. */
/* Insert functions do not modify cur.
 * Insert functions do not allocate a new copy of the item.
 * Delete functions move cur to the next node when possible. */

/* Push an item onto the end of the list */
int listPush(linkedlist * list, listitem item);

/* Insert an item before cur
 * When cur == NULL, item will be inserted at the end of the list */
int listInsert(linkedlist * list, listitem item);

/* Remove the current item, returning the item */
listitem listRemoveItem(linkedlist * list);

/* Delete (free) the current item */
int listDeleteItem(linkedlist * list);

/* Remove all items from the list, but do not free any of them */
void listRemoveAll(linkedlist * list);

/* Delete (free) all items in the list */
int listDeleteAll(linkedlist * list);


/* Duplicate a list. Exact copies of each item will be allocated. */
linkedlist listDuplicate(linkedlist list);

/* Concatinate two lists. Both lists will then be identical (except cur). */
linkedlist * listCat(linkedlist * list1, linkedlist * list2);


/* Returns true if the list is empty */
int listIsEmpty(linkedlist* list);

/* Get the current item in the list */
listitem listGetItem(linkedlist * list);

/* Move cur to the first element in the list */
void listStart(linkedlist* list);

/* Advance to the next item in the list
 * Returns true on success, false when the end of the list has been reached */
int listAdvance(linkedlist* list);

/* Move cur forward or backward by a number of steps */
int listMoveBy(linkedlist* list, int delta);

/* Find the offset of cur from first */
int listGetPosition(linkedlist* list);


/* Item functions */

/* Allocate an exact copy of an item */
listitem duplicateItem(listitem src);

#endif
