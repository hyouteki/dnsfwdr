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
#define DNS_IP "8.8.8.8"
#define DNS_PORT 53

char *ForwardRequest(char *, int, int *);

char *ForwardRequest(char *buffer, int buffer_size, int *response_size) {
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		perror("Error: socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DNS_PORT);
    if (inet_pton(AF_INET, DNS_IP, &server_addr.sin_addr) <= 0) {
        perror("Error: IP invalid\n");
        exit(EXIT_FAILURE);
    }
	
    sendto(sockfd, buffer, buffer_size, 0,
		   (struct sockaddr *)&server_addr, sizeof(server_addr));

	char *dns_response_buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	memset(dns_response_buffer, 0, BUFFER_SIZE);
	
    struct sockaddr_in recv_addr;
	int recv_size = sizeof(recv_addr);
	*response_size = recvfrom(sockfd, dns_response_buffer, BUFFER_SIZE, 0,
							(struct sockaddr *)&recv_addr, &recv_size);
    if (!*response_size) {
        perror("Error: DNS response receive error\n");
        exit(EXIT_FAILURE);
    }

	printf("|\tDNS RESPONSE RECEIVED\n");

    close(sockfd);
    return dns_response_buffer;
}

int main() {
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		perror("Error: socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("Error: socket bind failed\n");
		exit(EXIT_FAILURE);
	}

	char *buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer, 0, BUFFER_SIZE);
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
		printf("|\tHeader(id=%d, qdcount=%d, ancount=%d nscount=%d arcount=%d)\n",
			   header.id, header.qdcount, header.ancount, header.nscount, header.arcount);

		Dns_Question question = Dns_ParseQuestion(buffer);
		printf("|\tQuestion(qname=%s, qtype=%d, qclass=%d)\n",
			   question.qname, question.qtype, question.qclass);
		
		printf("|\tFORWARDING REQUEST\n");

		int response_size;
		char *dns_response = ForwardRequest(buffer, recv_len, &response_size);

		if (sendto(sockfd, dns_response, response_size, 0,
				   (struct sockaddr *)&client_addr, client_len) < 0) {
			close(sockfd);
			perror("Error: could not send DNS response back to client\n");
			exit(EXIT_FAILURE);
		}
		printf("|\tDNS RESPONSE SENT\n");
		
    }
	
	close(sockfd);
	return 0;
}
