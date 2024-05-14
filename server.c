#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dns.h"
#include "dns_cache.h"
#include "helper.h"

#define Port 8989
#define Dns_Ip "8.8.8.8"
#define Dns_Port 53

Buffer ForwardRequest(Buffer);

Buffer ForwardRequest(Buffer query) {
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (!sockfd) Error("socket creation failed");

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Dns_Port);
    if (!inet_pton(AF_INET, Dns_Ip, &server_addr.sin_addr))
		Error("IP invalid");
	
    if (!sendto(sockfd, query.buffer, query.size, 0,
			   (struct sockaddr *)&server_addr, sizeof(server_addr)))
		Error("send failed");
		
	Buffer response = Buffer_Init();	
    struct sockaddr_in recv_addr;
	int recv_size = sizeof(recv_addr);
	response.size = recvfrom(sockfd, response.buffer, Buffer_Size, 0,
							 (struct sockaddr *)&recv_addr, &recv_size);
    if (!response.size) Error("DNS response receive error");
	printf("|\tRESPONSE RECEIVED\n");

    close(sockfd);
    return response;
}

int main() {
	Dns_Cache cache = {0};
	
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (!sockfd) Error("socket creation failed");

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(Port);
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		Error("socket bind failed");

	Buffer query = Buffer_Init();
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        query.size = recvfrom(sockfd, query.buffer, Buffer_Size, 0,
							  (struct sockaddr *)&client_addr, &client_len);
        if (!query.size) {
            close(sockfd);
            Error("receive failed");
        }

        printf("Received(%dB) From(%s:%d)\n", query.size,
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        Dns_Header header = Dns_ParseHeader(query.buffer);
		printf("|\tHeader(id=%d, qdcount=%d, ancount=%d nscount=%d arcount=%d)\n",
			   header.id, header.qdcount, header.ancount, header.nscount, header.arcount);

		Dns_Question question = Dns_ParseQuestion(query.buffer);
		printf("|\tQuestion(qname=%s, qtype=%d, qclass=%d)\n",
			   question.qname, question.qtype, question.qclass);

		Buffer response;
		if (Dns_CacheFind(&cache, question.qname)) {
			printf("|\tCACHE HIT\n");
			response = Dns_CacheLookup(&cache, question.qname);
			strncpy(response.buffer, query.buffer, Dns_HeaderLen);
		} else {
			printf("|\tREQUEST FORWARDED\n");
			response = ForwardRequest(query);
			Dns_CacheInsert(&cache, question.qname, response);
		}
		
		if (sendto(sockfd, response.buffer, response.size, 0,
				   (struct sockaddr *)&client_addr, client_len) < 0) {
			close(sockfd);
			Error("could not send DNS response back to client");
		}
		printf("|\tRESPONSE SENT\n");
    }
	
	close(sockfd);
	return 0;
}
