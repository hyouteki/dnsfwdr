#ifndef DNSFWDR_DNS_CACHE_H_
#define DNSFWDR_DNS_CACHE_H_

#define Dns_CacheSize 2048

typedef struct Dns_Entry {
	char *query;
	char *response;
	char *additional;
} Dns_Entry;

typedef struct Dns_Cache {
	Dns_Entry table[Dns_MaxCacheSize];
	int size;
} Dns_Cache;

unsigned int Dns_Hash(char *);
void Dns_CacheInsert(Dns_Cache *, Dns_Entry);

unsigned int Dns_Hash(char *buffer) {
	char *itr = buffer;
	unsigned long long hash = 0;
	while (itr) {
		hash = *itr + hash<<6 + hash<<16 - hash;
		itr++;
	}
	return hash%Dns_CacheSize;
}

#endif // DNSFWDR_DNS_CACHE_H_
