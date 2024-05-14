#ifndef DNSFWDR_DNS_CACHE_H_
#define DNSFWDR_DNS_CACHE_H_

#include "helper.h"
#include "string.h"

#define Dns_CacheSize 2048

typedef struct Dns_Entry {
	char *query;
	Buffer response;
	struct Dns_Entry *next;
} Dns_Entry;

typedef struct Dns_Cache {
	Dns_Entry *table[Dns_CacheSize];
	int size;
} Dns_Cache;

unsigned int Dns_Hash(char *);
void Dns_CacheInsert(Dns_Cache *, char *, Buffer);
int Dns_CacheFind(Dns_Cache *, char *);
Buffer Dns_CacheLookup(Dns_Cache *, char *);

// sdbm hash
unsigned int Dns_Hash(char *buffer) {
	unsigned long long hash = 0;
	for (int i = 0; i < strlen(buffer); ++i)
		hash = buffer[i] + hash<<6 + hash<<16 - hash;
	return hash%Dns_CacheSize;
}

void Dns_CacheInsert(Dns_Cache *cache, char *query, Buffer response) {
	unsigned int hash = Dns_Hash(query);
	Dns_Entry *entry = (Dns_Entry *)malloc(sizeof(Dns_Entry));
    if (!entry) Error("cache entry allocation failed");
    entry->query = strdup(query);
    entry->response = Buffer_Dup(response);
	entry->next = cache->table[hash];
    cache->table[hash] = entry;
}

int Dns_CacheFind(Dns_Cache *cache, char *query) {
	unsigned int hash = Dns_Hash(query);
	Dns_Entry *itr = cache->table[hash];
	while (itr) {
		if (strcmp(itr->query, query) == 0) return 1;
		itr = itr->next;
	}
	return 0;
}

// unsafe lookup assuming Dns_Find returned 1
Buffer Dns_CacheLookup(Dns_Cache *cache, char *query) {
	unsigned int hash = Dns_Hash(query);
	Dns_Entry *itr = cache->table[hash];
	while (itr) {
		if (strcmp(itr->query, query) == 0)
			return Buffer_Dup(itr->response);
		itr = itr->next;
	}
}

#endif // DNSFWDR_DNS_CACHE_H_
