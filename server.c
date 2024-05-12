#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dns.h"

#define BUFFER_SIZE 1024
#define PORT 8989

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void parse_dns_query(const char *buffer, int buflen) {
    // DNS header is 12 bytes
    // Skipping header for now
    int pos = 12;

    // Buffer to store domain name
    char domain_name[256];
    memset(domain_name, 0, sizeof(domain_name));
    int domain_pos = 0;

    // Extracting domain name from question section
    while (pos < buflen) {
        int label_len = buffer[pos++];

        if (label_len == 0) {
            // End of domain name
            break;
        } else if (label_len >= 192) {
            // Pointer, skip to another offset
            pos++;
            break;
        }

        // Copy label to domain_name buffer
        for (int i = 0; i < label_len; i++) {
            domain_name[domain_pos++] = buffer[pos++];
        }
        domain_name[domain_pos++] = '.';

        // Check for next label
        if (buffer[pos] == 0) {
            // End of domain name
            pos++;
            break;
        }
    }

    // Skipping QTYPE and QCLASS
    pos += 4;

    // Print the parsed domain name
    printf("Parsed domain name: %s\n", domain_name);
}

int main() {
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		perror("Error: socket creation failed\n");
		exit(EXIT_FAILURE);
	}
	printf("Debug: socket created\n");

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("Error: socket bind failed\n");
		exit(EXIT_FAILURE);
	}
	printf("Debug: socket bind\n");
	printf("Debug: UDP server running ...\n");

	char buffer[BUFFER_SIZE];
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                                (struct sockaddr *)&client_addr, &client_len);
        if (recv_len < 0) {
            close(sockfd);
            perror("Error: receive failed");
            exit(EXIT_FAILURE);
        }

        printf("Log: received(%d) bytes from %s:%d\n", recv_len,
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        Dns_Header header = Dns_ParseHeader(buffer);
		printf("Log: Header(id=%d, qdcount=%d, ancount=%d nscount=%d arcount=%d)\n",
			   header.id, header.qdcount, header.ancount, header.nscount, header.arcount);

		Dns_Question question = Dns_ParseQuestion(buffer);
		printf("Log: Question(qname=%s, qtype=%d, qclass=%d)\n",
			   question.qname, question.qtype, question.qclass);
		parse_dns_query(buffer, recv_len);
    }
	
	close(sockfd);
	return 0;
}
