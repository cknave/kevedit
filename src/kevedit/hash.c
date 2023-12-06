/* hash.c       -- Rudimentary hash table for bind index merging
 * $Id: hash.c,v 1.3 2023/11/12 18:09:18 kristomu Exp $
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

#include "hash.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

hash_table hashInit(int max_items)
{
	/* Initialize an array of size twice the number of items as a
	 * conservative estimate. */
	hash_table out;

	out.size = 2 * max_items;
	out.table = malloc(sizeof(llnode *) * out.size);
	memset(out.table, 0, sizeof(llnode *) * out.size);

	return out;
}

/* Adds the node to the end. */
void addNode(hash_table * htab, ZZTparam * param)
{
	llnode * new_node = malloc(sizeof(llnode));
	memset(new_node, 0, sizeof(llnode));

	new_node->param = param;

	int table_idx = param->program_hash % htab->size;

	/* If there's no chain at this part of the hash table, just
	 * insert a new one. Otherwise add it to the end of the
	 * chain.*/

	if (htab->table[table_idx] == NULL) {
		new_node->previous = new_node;
		new_node->next = NULL;
		htab->table[table_idx] = new_node;
	} else {
		llnode * last_node = htab->table[table_idx]->previous;

		assert(last_node->next == NULL);
		last_node->next = new_node;
		new_node->previous = last_node;
		new_node->next = NULL;

		 /* update last node reference */
		htab->table[table_idx]->previous = new_node;
	}
}

void addNodes(hash_table * htab, ZZTblock * block)
{
	int i;

	for (i = 1; i < block->paramcount; ++i) {
		ZZTparam * param = block->params[i];

		if (param->length > 0) {
			addNode(htab, param);
		}
	}
}

void removeNode(hash_table * htab, llnode * node)
{
	/* We first need to check if the node is the first in its chain,
	 * which requires looking it up. */

	int table_idx = node->param->program_hash % htab->size;

	/* If we're the only node in the chain, just set the table
	 * entry to null and free it. */
	if (node->previous == node) {
		assert(htab->table[table_idx] == node);
		htab->table[table_idx] = NULL;
		free(node);
		return;
	}

	/* If we're the first node in the chain, update the
	 * next's previous to our previous and set it to
	 * the new first node. This will be the last node.
	 * The last's next doesn't need to be set since it
	 * being null indicates the end of the chain. */
	if (htab->table[table_idx] == node) {
		node->next->previous = node->previous;
		htab->table[table_idx] = node->next;
		free(node);
		return;
	}

	/* If we're the last node in the chain, update the previous's
	 * next and set the last node reference on the first node in
	 * the chain. */

	if (htab->table[table_idx]->previous == node) {
		assert(node->next == NULL);
		assert(node->previous != NULL);
		node->previous->next = NULL;
		htab->table[table_idx]->previous = node->previous;
		free(node);
		return;
	}

	/* If we're not, set our next's previous and previous' next. */
	node->next->previous = node->previous;
	node->previous->next = node->next;
	free(node);
}

const llnode * getFirstByHash(hash_table * htab, ZZTparam * param)
{
	int table_idx = param->program_hash % htab->size;

	return htab->table[table_idx];
}

int isProgramEqual(ZZTparam * a, ZZTparam * b) {
	return (a->length == b->length)
		&& (memcmp(a->program, b->program, a->length) == 0);
}

const llnode * getFirstEqual(hash_table * htab, ZZTparam * param)
{
	int table_idx = param->program_hash % htab->size;
	llnode * candidate = htab->table[table_idx];

	/* Keep advancing as long as the candidate isn't NULL and
	 * either the program length or contents differ. */

	while (candidate != NULL
		&& !isProgramEqual(param, candidate->param)) {

		candidate = candidate->next;
	}

	return candidate;
}

const llnode * getNextEqual(const llnode * last, ZZTparam * param)
{
	if (last == NULL) {
		return NULL;
	}

	const llnode * next_node = last;

	do {
		next_node = next_node -> next;
	} while (next_node != NULL
		&& !isProgramEqual(param, next_node->param));

	return next_node;
}

void freeTable(hash_table * htab) {

	int i;

	/* Free every node in the table. */
	for (i = 0; i < htab->size; ++i) {
		while (htab->table[i] != NULL) {
			removeNode(htab, htab->table[i]);
		}
	}

	/* Free the table itself. */
	free(htab->table);
	htab->table = NULL;
	htab->size = 0;
}