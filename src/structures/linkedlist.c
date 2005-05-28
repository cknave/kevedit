/**@file structures/linkedlist.c   An abstract linked list.
 * $Id: linkedlist.c,v 1.4 2005/05/28 03:17:45 bitman Exp $
 * @author Ryan Phillips
 *
 * Copyright (C) 2003 Ryan Phillips <bitman@users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "linkedlist.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/** An empty item. */
const listitem EMPTY_ITEM = { 0, NULL };

/**
 * @relates linkedlist
 * @brief Initialize a linked list.
 *
 * @param list  an uninitialized list.
 *
 * Always initialize a linked list before using it.
 **/
void initlinkedlist(linkedlist * list)
{
	list->first = list->last = list->cur = NULL;
}


/**
 * @relates linkedlist
 * @brief Push an item onto the end of the list.
 *
 * @param list  a list.
 * @param item  item to push.
 *
 * @return 0 on success, 1 on error.
 **/
int listPush(linkedlist * list, listitem item)
{
	link *newnode = NULL;

	if (list == NULL)
		return 1;

	newnode = (link *) malloc(sizeof(link));
	newnode->item = item;

	if (listIsEmpty(list)) {
		/* List is empty, add item as only node */
		newnode->next = NULL;
		newnode->prev = NULL;

		list->last = list->first = newnode;
	} else {
		/* Add item after last node */
		newnode->prev = list->last;
		newnode->next = NULL;
		list->last->next = newnode;
		list->last       = newnode;
	}

	return 0;
}

/**
 * @relates linkedlist
 * @brief Insert an item.
 *
 * @param list  a list.
 * @param item  item to insert
 *
 * Item is inserted before @c cur. Use <code>list.cur = NULL</code> to insert
 * at the end of the list.
 *
 * @return 0 on success, 1 on error.
 **/
int listInsert(linkedlist * list, listitem item)
{
	link *newnode = NULL;

	if (list == NULL)
		return 1;

	if (listIsEmpty(list))
		return listPush(list, item);

	newnode = (link *) malloc(sizeof(link));
	newnode->item = item;
	
	/* If cur is NULL, insert after last item */
	if (list->cur == NULL) {
		newnode->prev = list->last;
		newnode->next = NULL;
		list->last = newnode;
	} else {
		/* Otherwise insert before current item */
		newnode->prev = list->cur->prev;
		newnode->next = list->cur;
		newnode->next->prev = newnode;
	}

	/* If we are at the start of the list, newnode->prev may be NULL */
	if (newnode->prev != NULL)
		newnode->prev->next = newnode;
	else if (list->first == list->cur)
		list->first = newnode;
	else
		return 1;

	return 0;
}

/**
 * @relates linkedlist
 * @brief Remove the current item.
 *
 * @param list  a list.
 *
 * @return the removed item.
 **/
listitem listRemoveItem(linkedlist * list)
{
	listitem item;
	link *cur;

	if (list == NULL || list->cur == NULL)
		return EMPTY_ITEM;

	item = list->cur->item;
	cur = list->cur;

	/* if there is a previous node link it to the next node. If cur
	 * is at the end of the vector, then the previous will point to
	 * NULL, which is okay. */
	if (cur->prev != NULL)
		cur->prev->next = cur->next;

	/* else if the current node is not first, list is messed up */
	else if (cur != list->first) {
		return EMPTY_ITEM;
	}

	/* now since the cur node is first, move first to value of next node,
	 * which may be NULL if cur is the only item in the list. */
	else
		list->first = cur->next;

	/* list->cur will either be set to the next node or the last node,
	 * depending on whether cur is at the end of the list */

	/* if there is a next node, link it backward to the previous node */
	if (cur->next != NULL) {
		cur->next->prev = cur->prev;
		list->cur = cur->next;
	}
	/* else if the current node is not last, list is messed up */
	else if (cur != list->last)
		return EMPTY_ITEM;

	/* now that the cur node is also last, move last to value of prev node,
	 * which may be NULL if cur is the only item in the vector. */
	else
		list->last = list->cur = cur->prev;

	/* now we can get rid of the current node */
	free(cur);

	return item;
}

/**
 * @relates linkedlist
 * @brief Delete (free) the current item.
 *
 * @param list  a list.
 *
 * @return 0 on success, 1 on error.
 **/
int listDeleteItem(linkedlist * list)
{
	void *value = listRemoveItem(list).value;

	if (value == NULL)
		return 1;

	free(value);

	return 0;
}

/**
 * @relates linkedlist
 * @brief Remove all items without freeing them.
 *
 * @param list  a list.
 *
 **/
void listRemoveAll(linkedlist * list)
{
	if (list == NULL)
		return;

	list->cur = list->last;
	while (!listIsEmpty(list))
		listRemoveItem(list).value;

	return;
}

/**
 * @relates linkedlist
 * @brief Delete (free) all items.
 *
 * @param list  
 *
 * All items in the list will be freed.
 *
 * @return 0 on success, 1 on error.
 **/
int listDeleteAll(linkedlist * list)
{
	if (list == NULL)
		return 1;

	list->cur = list->last;
	while (listDeleteItem(list) != 1);

	return 0;
}

/**
 * @relates linkedlist
 * @brief Duplicate a list.
 *
 * @param list  
 *
 * Exact copies of each item will be allocated.
 *
 * @warning If an item's @c data contains pointer(s) to more data, that data
 * will not be duplicated.
 *
 * @return a copy of @c list.
 **/
linkedlist listDuplicate(linkedlist list)
{
	link * iterator = NULL;
	linkedlist dup;
	initlinkedlist(&dup);

	for (iterator = list.first; iterator != NULL; iterator = iterator->next) {
		listitem copy = duplicateItem(iterator->item);
		listPush(&dup, copy);
	}

#if 0
	for (listResetCur(&list); list.cur != NULL; list.cur = list.cur->next) {
		void* valuedup = malloc(list.cur->size);
		memcpy(valuedup, list.cur->value, list.cur->size);
		listPush(&dup, valuedup);
	}
#endif

	return dup;
}

/**
 * @relates linkedlist
 * @brief Concatinate two lists.
 *
 * @param list1  first list.
 * @param list2  second list.
 *
 * @c list2 will be appened to the end of @c list1. Both lists will share the
 * same items and thus be identical, with the exception of @c cur.
 *
 * @warning Do not continue to use both @c list1 and @c list2 if any changes
 * are made to the list structure. 
 *
 * @return The now concatinated @c list1.
 **/
linkedlist * listCat(linkedlist * list1, linkedlist * list2)
{
	if (listIsEmpty(list1)) {
		*list1 = *list2;
	} else {
		/* If list1's cur points to the end of the list, switch it to the first
		 * item in list2 */
		if (list1->cur == NULL)
			list1->cur = list2->first;

		/* Connect the lists */
		list1->last->next       = list2->first;
		list2->first->prev      = list1->last;
		list1->last             = list2->last;
		list2->first            = list1->first;
	}

	return list1;
}

/**
 * @relates linkedlist
 * @brief Check whether list is empty.
 *
 * @param list  a list.
 *
 * @return true only if the list is empty.
 **/
int listIsEmpty(linkedlist* list)
{
	return list->first == NULL || list->last == NULL;
}

/**
 * @relates linkedlist
 * @brief Get the current item.
 *
 * @param list  a list.
 *
 * @return the current item.
 **/
listitem listGetItem(linkedlist * list)
{
	if (list != NULL && list->cur != NULL)
		return list->cur->item;
	else
		return EMPTY_ITEM;
}

/**
 * @relates linkedlist
 * @brief Move to the first item in the list.
 *
 * @param list  a list.
 **/
void listStart(linkedlist* list)
{
	list->cur = list->first;
}

/**
 * @relates linkedlist
 * @brief Advance to the next item in the list.
 *
 * @param list  a list.
 *
 * @return true on success, false when the end of the list has been reached.
 **/
int listAdvance(linkedlist* list)
{
	/* Don't even bother advancing if we've reached the end */
	if (list->cur == NULL)
		return 0;

	/* Advance */
	list->cur = list->cur->next;

	/* Return false unless we've reached the end of the list */
	if (list->cur == NULL)
		return 0;
	else
		return 1;
}

/**
 * @relates linkedlist
 * @brief Move forward or backward by a number of items.
 *
 * @param list   a list.
 * @param delta  number of steps to move.
 *
 * Positive values for @c delta will move @c cur forward, negative values
 * will move backward.
 *
 * @return number of items actually traversed.
 **/
int listMoveBy(linkedlist* list, int delta)
{
	int i;
	
	if (delta > 0) {
		for (i = 0; i < delta && list->cur->next != NULL; i++)
			list->cur = list->cur->next;
		return i;
	} else {
		for (i = 0; i > delta && list->cur->prev != NULL; i--)
			list->cur = list->cur->prev;
		return -i;
	}
}

/**
 * @relates linkedlist
 * @brief Get the current position.
 *
 * @param list  a list.
 *
 * @return offset of @c cur from the beginning of the list.
 **/
int listGetPosition(linkedlist* list)
{
	int pos = 0;
	link *finder = list->first;

	while (finder != list->cur && finder != NULL) {
		finder = finder->next;
		pos++;
	}

	return pos;
}


/* Item functions */

/**
 * @relates listitem
 * @brief Duplicate an item.
 *
 * @param src  item to duplicate.
 *
 * @return an exact allocated copy of @c src.
 **/
listitem duplicateItem(listitem src)
{
	listitem dest;

	dest.size = src.size;
	dest.value = (void*) malloc(src.size);
	if (dest.value == NULL) {
		dest.size = 0;
		return dest;
	}

	memcpy(dest.value, src.value, src.size);
	return dest;
}

