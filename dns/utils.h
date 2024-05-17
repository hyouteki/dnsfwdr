#ifndef DNSFWDR_DNS_UTILS_H_
#define DNSFWDR_DNS_UTILS_H_

#include <string.h>
#include <ctype.h>

#define Buffer_Size 1024
#define Error(message) ({fprintf(stderr, "Error: %s\n", message); \
			exit(EXIT_FAILURE);}) 

typedef struct Buffer {
	char *buffer;
	int size;
} Buffer;

Buffer Buffer_Init();
Buffer Buffer_Dup(Buffer);
unsigned int Dns_Hash(char *);

Buffer Buffer_Init() {
	char *buffer = (char *)malloc(sizeof(char)*Buffer_Size);
	memset(buffer, 0, Buffer_Size);
	return (Buffer){.buffer=buffer, .size=0};
}

Buffer Buffer_Dup(Buffer buffer) {
	return (Buffer){.buffer=strdup(buffer.buffer), .size=buffer.size};
}

// sdbm hash
unsigned int Dns_Hash(char *buffer) {
	unsigned long long hash = 0;
	for (int i = 0; i < strlen(buffer); ++i)
		hash = buffer[i] + hash<<6 + hash<<16 - hash;
	return hash;
}

char *Dns_StrTrim(char *buffer) {
    while (isspace(*buffer)) buffer++;
    if (!*buffer) return buffer;
    char *end = buffer + strlen(buffer) - 1;
    while (end > buffer && isspace(*end)) end--;
    *(end + 1) = 0;
    return buffer;
}

#endif // DNSFWDR_DNS_UTILS_H_
