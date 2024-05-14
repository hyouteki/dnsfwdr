#ifndef DNSFWDR_HELPER_H_
#define DNSFWDR_HELPER_H_

#include "string.h"

#define Buffer_Size 1024
#define Error(message) ({fprintf(stderr, "Error: %s\n", message); \
			exit(EXIT_FAILURE);}) 

typedef struct Buffer {
	char *buffer;
	int size;
} Buffer;

Buffer Buffer_Init();
Buffer Buffer_Dup(Buffer);

Buffer Buffer_Init() {
	char *buffer = (char *)malloc(sizeof(char)*Buffer_Size);
	memset(buffer, 0, Buffer_Size);
	return (Buffer){.buffer=buffer, .size=0};
}

Buffer Buffer_Dup(Buffer buffer) {
	return (Buffer){.buffer=strdup(buffer.buffer), .size=buffer.size};
}

#endif // DNSFWDR_HELPER_H_
