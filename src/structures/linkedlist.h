/**@file structures/linkedlist.h   An abstract linked list.
 * $Id: linkedlist.h,v 1.4 2003/12/20 09:12:21 bitman Exp $
 * @author Ryan Phillips
 *
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

#ifndef STRUCTURES_LINKEDLIST_H
#define STRUCTURES_LINKEDLIST_H

/**
 * A list item. The listitem structure keeps track of the data pointer and the
 * size of the data for each item in the list. You can cast as a listitem any
 * structure which has the form:
 *
 * @code
 * struct {
 * 	size_t size;
 * 	sometype* value;
 * };
 * @endcode
 *
 * This may be useful when providing a wrapper for a specific data type.
 */
typedef struct listitem {
	size_t size;         /**< Size of the data stored in @c value. */
	void *value;         /**< Item data. */
} listitem;

/** A link in a linkedlist. */
typedef struct link {
	listitem item;       /**< Item belonging to this link. */

	struct link *next;   /**< Next link. */
	struct link *prev;   /**< Previous link. */
} link;

/**
 * A linked list. An abstracted type that can store, duplicate, and free() data
 * of any type.
 */
typedef struct linkedlist {
	link *first;         /**< First item in the list. */
	link *last;          /**< Last item in the list. */

	/**
	 * Current item in the list. Determines where insert, remove, and delete
	 * operations will occur.
	 */
	link *cur;

} linkedlist;


extern const listitem EMPTY_ITEM;

/* Linked list functions */

void initlinkedlist(linkedlist * list);

int listPush(linkedlist * list, listitem item);
int listInsert(linkedlist * list, listitem item);

listitem listRemoveItem(linkedlist * list);
int listDeleteItem(linkedlist * list);
void listRemoveAll(linkedlist * list);
int listDeleteAll(linkedlist * list);

linkedlist listDuplicate(linkedlist list);
linkedlist * listCat(linkedlist * list1, linkedlist * list2);

int listIsEmpty(linkedlist* list);
listitem listGetItem(linkedlist * list);

void listStart(linkedlist* list);
int listAdvance(linkedlist* list);

int listMoveBy(linkedlist* list, int delta);
int listGetPosition(linkedlist* list);


/* Item functions */

listitem duplicateItem(listitem src);

#endif
