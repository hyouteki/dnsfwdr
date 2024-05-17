#ifndef DNSFWDR_DNS_BLOCK_H_
#define DNSFWDR_DNS_BLOCK_H_

#include <unistd.h>
#include <string.h>
#include "utils.h"

#define Dns_BlockListSize 2048

typedef struct Block_Entry {
	char *query;
	struct Block_Entry *next;
} Block_Entry;

typedef struct Dns_BlockList {
	Block_Entry *table[Dns_BlockListSize];
} Dns_BlockList;

void Dns_BlockListInit(Dns_BlockList *, char *);
void Dns_BlockListInsert(Dns_BlockList *, char *);
int Dns_BlockListFind(Dns_BlockList *, char *);

void Dns_BlockListInit(Dns_BlockList *block_list, char *filepath) {
	FILE *file = fopen(filepath, "r");
	if (!file) Error("could not open file");

	char *line = NULL;
	size_t len = 0;
	while ((len = getline(&line, &len, file)) != -1) {
		if (strlen((line = Dns_StrTrim(line))))
			Dns_BlockListInsert(block_list, line);
	}

	fclose(file);
    if (line) free(line);
}

void Dns_BlockListInsert(Dns_BlockList *block_list, char *query) {
	unsigned int hash = Dns_Hash(query)%Dns_BlockListSize;
	Block_Entry *entry = (Block_Entry *)malloc(sizeof(Block_Entry));
    if (!entry) Error("block entry allocation failed");
    entry->query = strdup(query);
	entry->next = block_list->table[hash];
    block_list->table[hash] = entry;
}

int Dns_BlockListFind(Dns_BlockList *block_list, char *query) {
	unsigned int hash = Dns_Hash(query)%Dns_BlockListSize;
	Block_Entry *itr = block_list->table[hash];
	while (itr) {
		if (strcmp(itr->query, query) == 0) return 1;
		itr = itr->next;
	}
	return 0;
}

#endif // DNSFWDR_DNS_BLOCK_H_
