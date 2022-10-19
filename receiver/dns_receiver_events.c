#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "dns_receiver_events.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "../base32.h"
#include <sys/types.h>

#define NETADDR_STRLEN (INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN)
#define CREATE_IPV4STR(dst, src) char dst[NETADDR_STRLEN]; inet_ntop(AF_INET, src, dst, NETADDR_STRLEN)
#define CREATE_IPV6STR(dst, src) char dst[NETADDR_STRLEN]; inet_ntop(AF_INET6, src, dst, NETADDR_STRLEN)
#define BASE32_LENGTH_ENCODE(src_size) (((src_size)*8 + 4) / 5)
#define BASE32_LENGTH_DECODE(src_size) (ceil((src_size)) / 1.6)


void print_buffer(unsigned char *buffer, size_t len) {
  unsigned char preview[17];
  preview[16] = '\0';
  memset(preview, ' ', 16);
  for (int i = 0; i < len; ++i) {
    if (i && i % 16 == 0) {
      printf(" %s\n", preview);
      memset(preview, ' ', 16);
    }
    unsigned char c = buffer[i];
    printf("%02x ", c);
    preview[i % 16] = (c == ' ' || (c >= '!' && c < '~')) ? c : '.';
  }
  for (int i = 0; i < 16 - len % 16; ++i) {
    printf("   ");
  }
  printf(" %s\n", preview);
}

void ChangetoDnsNameFormat(char* dns, char* host) 
{
	int lock = 0 , i;
	strcat((char*)host,".");
	char help[5];
	
	for(i = 0 ; i < strlen((char*)host) ; i++) 
	{

		if(host[i]=='.') 
		{
			sprintf(help, "%x", i-lock);
			strcat((char*)dns, help);
			*dns++;
			for(;lock<i;lock++) 
			{
				*dns++=host[lock];
			}
			lock++;
		}
	}
	host[strlen((char *)host) - 1] = '\0';
	*dns++='0';
}

struct dataStruct
{
	char* inputData;
	int allocatedSpace;
};

void dns_receiver__on_query_parsed(char *filePath, char *encodedData)
{
	fprintf(stderr, "[PARS] %s '%s'\n", filePath, encodedData);
}

void on_chunk_received(char *source, char *filePath, int chunkId, int chunkSize)
{
	fprintf(stderr, "[RECV] %s %9d %dB from %s\n", filePath, chunkId, chunkSize, source);
}

void dns_receiver__on_chunk_received(struct in_addr *source, char *filePath, int chunkId, int chunkSize)
{
	CREATE_IPV4STR(address, source);
	on_chunk_received(address, filePath, chunkId, chunkSize);
}

void dns_receiver__on_chunk_received6(struct in6_addr *source, char *filePath, int chunkId, int chunkSize)
{
	CREATE_IPV6STR(address, source);
	on_chunk_received(address, filePath, chunkId, chunkSize);
}

void on_transfer_init(char *source)
{
	fprintf(stderr, "[INIT] %s\n", source);
}

void dns_receiver__on_transfer_init(struct in_addr *source)
{
	CREATE_IPV4STR(address, source);
	on_transfer_init(address);
}

void dns_receiver__on_transfer_init6(struct in6_addr *source)
{
	CREATE_IPV6STR(address, source);
	on_transfer_init(address);
}

void dns_receiver__on_transfer_completed(char *filePath, int fileSize)
{
	fprintf(stderr, "[CMPL] %s of %dB\n", filePath, fileSize);
}


int main(int argc, char *argv[]){


	if(argc != 3){
		fprintf(stderr, "Error: Wrong number of program arguments\n");
		exit(1);
	}

	char* BASE_HOST = argv[1];
	char* DST_FILEPATH = argv[2];

	// printf("%s, %s\n", BASE_HOST, DST_FILEPATH); 



	int serverSocket;
	unsigned char buffer[512];
	struct sockaddr_in serverAddr, clientAddr;

	if((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "ERROR: Failed to create socket\n");
		exit(1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr)); 
    memset(&clientAddr, 0, sizeof(clientAddr)); 

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(53);
	// serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	int optval = 1;	
	if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)))
	{
		fprintf(stderr, "ERROR in setsockopt function\n");
		exit(1);
	}

	int returnCode = 0;
	if((returnCode = bind(serverSocket, (const struct sockaddr *)&serverAddr, sizeof(serverAddr))) < 0){

		fprintf(stderr, "ERROR: Failed to bind, %s\n", strerror(errno));
		exit(1);
	}
	printf("Binding succesfull\n");

	int lenght = sizeof(clientAddr);
	int numOfBytesReceived = 0;
	while(true){
		memset(buffer, '\0', sizeof(buffer));
		numOfBytesReceived = recvfrom(serverSocket, (unsigned char *)buffer, 512, MSG_WAITALL , (struct sockaddr *)&clientAddr, &lenght);
		printf("Client: %s\n", buffer);
		printf("Bytes: %i\n", numOfBytesReceived);

		unsigned char client_addr_str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(clientAddr.sin_addr), client_addr_str, INET_ADDRSTRLEN);
		printf("---------------------------\nReceived %d bytes from %s\n",
           numOfBytesReceived, client_addr_str);


		print_buffer(buffer, numOfBytesReceived);

		struct DNS_HEADER *header = (struct DNS_HEADER *)buffer;
		// printf("header id: %d\n", header->id);

		char qname [253];
		memset(qname, '\0', 253);
		char *dns_query = (unsigned char*)&buffer[sizeof(struct DNS_HEADER)];
		// printf("dns_quey: %s\n", dns_query);
		ChangetoDnsNameFormat(qname, BASE_HOST);
		// printf("BASE_HOST: %s\n", BASE_HOST);
		// printf("qname: %s\n", qname);
		char *index = strstr(dns_query, qname);
		
		int iteration = 0;
		char data[253];
		memset(data, '\0', 253);
		while(&(dns_query[iteration]) != index){
			// printf("%c", dns_query[iteration]);
			data[iteration] = dns_query[iteration];
			
			iteration++;

		}
		printf("DATA: %s\n", data);

		
	}


	





	return 0;
}
