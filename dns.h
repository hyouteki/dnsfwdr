#ifndef DNSFWDR_DNS_H_
#define DNSFWDR_DNS_H_

#include <stdio.h>

#define Dns_Query 0
#define Dns_Response 1

#define Dns_OpcodeStandard 0
#define Dns_OpcodeInverse 1
#define Dns_OpcodeServerStatus 2

#define Dns_ResponseNoError 0
#define Dns_ResponseNameEror 3
#define Dns_ResponseServerFailure 5

#define Dns_QuestionMaxLen 256

typedef struct Dns_Header {
	unsigned short id;      // 16 bit  2
	short qr;               //  1 bit  3
	short opcode;           //  4 bit  3
	short aa;               //  1 bit  3
	short tc;               //  1 bit  3
	short rd;               //  1 bit  3
	short ra;               //  1 bit  4
	short z;                //  3 bit  4 (must be 0)
	short rcode;            //  4 bit  4
	unsigned int qdcount;   // 16 bit  6
	unsigned int ancount;   // 16 bit  8
	unsigned int nscount;   // 16 bit 10
	unsigned int arcount;   // 16 bit 12
} Dns_Header;

typedef struct Dns_Question {
	char *qname;
	unsigned short qtype;
	unsigned short qclass;
} Dns_Question;

static short Dns_ReadBit(char *, int);

Dns_Header Dns_ParseHeader(char *);
Dns_Question Dns_ParseQuestion(char *);

static short Dns_ReadBit(char *ch, int num) {
	if (num < 0 || num > 8) {
		perror("Error: number of bits to be read must be between [0, 8]\n");
		exit(EXIT_FAILURE);
	}
	short rop = 0;
	for (int i = 0; i < num; ++i) {
		rop = rop<<1 + *ch&1;
		*ch >>= 1;
	}
	return rop;
}

Dns_Header Dns_ParseHeader(char *buffer) {
	int itr = 0;
	unsigned short id = buffer[itr++];
	id << 8; id += buffer[itr++];
	
	char ch = buffer[itr++];
	short rd = Dns_ReadBit(&ch, 1);
	short tc = Dns_ReadBit(&ch, 1);
	short aa = Dns_ReadBit(&ch, 1);
	short opcode = Dns_ReadBit(&ch, 4);
	short qr = Dns_ReadBit(&ch, 1);

	ch = buffer[itr++];
	short rcode = Dns_ReadBit(&ch, 4);
	short z = Dns_ReadBit(&ch, 3);
	short ra = Dns_ReadBit(&ch, 1);

	if (z != 0) {
		perror("Error: corrupted DNS query; z (reserved) != 0\n");
 		exit(EXIT_FAILURE);
	}

	unsigned short qdcount = buffer[itr++];
	qdcount << 8; qdcount += buffer[itr++];

	unsigned short ancount = buffer[itr++];
	ancount << 8; ancount += buffer[itr++];

	unsigned short nscount = buffer[itr++];
	nscount << 8; nscount += buffer[itr++];

	unsigned short arcount = buffer[itr++];
	arcount << 8; arcount += buffer[itr++];
	
	return (Dns_Header){.id=id, .qr=qr, .opcode=opcode, .aa=aa, .tc=tc, .rd=rd,
						.ra=ra, .z=z, .rcode=rcode, .qdcount=qdcount, .ancount=ancount,
						.nscount=nscount, .arcount=arcount};
}

Dns_Question Dns_ParseQuestion(char *buffer) {	
    char qname[Dns_QuestionMaxLen];
    memset(qname, 0, sizeof(qname));
	
    int itr = 12, qnameItr = 0;

    while (buffer[itr]) {
        int len = buffer[itr++];

        if (!len) break;
        else if (len >= 192) {
            itr++;
            break;
        }

        for (int i = 0; i < len; i++) qname[qnameItr++] = buffer[itr++];
        if (buffer[itr]) qname[qnameItr++] = '.';
		else {
            itr++;
            break;
        }
    }

	
	unsigned short qtype = buffer[itr++];
	qtype << 8; qtype += buffer[itr++];
	
	unsigned short qclass = buffer[itr++];
	qclass << 8; qclass += buffer[itr++];

    return (Dns_Question){.qname=qname, .qtype=qtype, .qclass=qclass};
}

#endif // DNSFWDR_DNS_H_
