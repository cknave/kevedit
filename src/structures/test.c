/* Test the text handling library
 * $Id: test.c,v 1.2 2005/05/28 03:17:45 bitman Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

listitem stringitem(char * s) {
	listitem item;
	item.value = (void *) s;
	item.size = (strlen(s) + 1) * sizeof(char);
	return item;
}

listitem stringdupitem(char * s) {
	char * copy;
	copy = (char *) malloc(sizeof(char) * (strlen(s) + 1));
	strcpy(copy, s);
	return stringitem(copy);
}

void printlist(linkedlist list) {
	listStart(&list);
	do {
		printf("%s\n", (char *) listGetItem(&list).value);
	} while (listAdvance(&list));
	printf("\n");
}

int testLinkedList()
{
	linkedlist foobar, baaz;
	initlinkedlist(&foobar);

	/* Fill list with unallocated strings and print them */

	listPush(&foobar, stringitem("Greetings"));
	listStart(&foobar);
	listPush(&foobar, stringitem("Farewell"));
	listInsert(&foobar, stringitem("Vale"));
	listInsert(&foobar, stringitem("Salve"));

	printf("Unallocated list:\n");
	printlist(foobar);

	listRemoveAll(&foobar);

	/* Now fill with allocated strings */

	listInsert(&foobar, stringdupitem("Yeah yeah"));
	listInsert(&foobar, stringdupitem("Quaax"));
	listPush(&foobar, stringdupitem("Box"));

	printf("Allocated list:\n");
	printlist(foobar);

	printf("Duplicating list...\n");
	baaz = listDuplicate(foobar);

	printf("Concatinating lists...\n");
	listCat(&foobar, &baaz);

	printf("Deleting an item...\n");
	listDeleteItem(&foobar);

	printlist(foobar);

	printf("Deleting list...\n");
	listDeleteAll(&foobar);

	return 0;
}

int testDuplicateItem()
{
	listitem bob, sue;
	char* text;
	size_t size = sizeof(char) * 10;

	text = (char*) malloc(size);
	strcpy(text, "Hello");

	bob.value = text;
	bob.size = size;

	sue = duplicateItem(bob);

	printf("Sue: %s\n", (char*) sue.value);

	free(bob.value);
	free(sue.value);

	return 0;
}

int main()
{
	return testLinkedList();
}

