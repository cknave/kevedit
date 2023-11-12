#include "hash.h"

#include <stdlib.h>
#include <assert.h>

hash_table hashInit(int max_items)
{
	/* Initialize an array of size twice the number of items as a
	 * conservative estimate. */
	hash_table out;

	out.size = 2 * max_items;
	out.table = malloc(sizeof(llnode *) * max_items);
	memset(out.table, 0, sizeof(llnode *) * max_items);
}

/* Adds the node to the end. */
void addNode(hash_table * htab, ZZTparam * param, int param_index)
{
	llnode * new_node = malloc(sizeof(llnode));
	memset(new_node, 0, sizeof(llnode));

	new_node->param_index = param_index;
	new_node->param = param;

	int table_idx = param->program_hash % htab->size;

	/* If there's no chain at this part of the hash table, just
	 * insert a new one. Otherwise add it to the end of the
	 * chain.*/

	if (htab->table[table_idx] == NULL) {
		new_node->previous = new_node;
		new_node->next = NULL;
	} else {
		llnode * last_node = htab->table[table_idx]->previous;

		assert(last_node->next == NULL);
		last_node->next = new_node;
		new_node->previous = last_node;
		new_node->next = NULL;
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

	/* If we're not, set our next's previous and previous' next. */
	node->next->previous = node->previous;
	node->previous->next = node->next;
	free(node);
}

const llnode * getChainByHash(hash_table * htab, ZZTparam * param)
{
	int table_idx = param->program_hash % htab->size;

	return htab->table[table_idx];
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