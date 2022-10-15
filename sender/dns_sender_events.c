#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "dns_sender_events.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define NETADDR_STRLEN (INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN)
#define CREATE_IPV4STR(dst, src) char dst[NETADDR_STRLEN]; inet_ntop(AF_INET, src, dst, NETADDR_STRLEN)
#define CREATE_IPV6STR(dst, src) char dst[NETADDR_STRLEN]; inet_ntop(AF_INET6, src, dst, NETADDR_STRLEN)


void dns_sender__on_chunk_encoded(char *filePath, int chunkId, char *encodedData)
{
	fprintf(stderr, "[ENCD] %s %9d '%s'\n", filePath, chunkId, encodedData);
}

void on_chunk_sent(char *source, char *filePath, int chunkId, int chunkSize)
{
	fprintf(stderr, "[SENT] %s %9d %dB to %s\n", filePath, chunkId, chunkSize, source);
}

void dns_sender__on_chunk_sent(struct in_addr *dest, char *filePath, int chunkId, int chunkSize)
{
	CREATE_IPV4STR(address, dest);
	on_chunk_sent(address, filePath, chunkId, chunkSize);
}

void dns_sender__on_chunk_sent6(struct in6_addr *dest, char *filePath, int chunkId, int chunkSize)
{
	CREATE_IPV6STR(address, dest);
	on_chunk_sent(address, filePath, chunkId, chunkSize);
}

void on_transfer_init(char *source)
{
	fprintf(stderr, "[INIT] %s\n", source);
}

void dns_sender__on_transfer_init(struct in_addr *dest)
{
	CREATE_IPV4STR(address, dest);
	on_transfer_init(address);
}

void dns_sender__on_transfer_init6(struct in6_addr *dest)
{
	CREATE_IPV6STR(address, dest);
	on_transfer_init(address);
}

void dns_sender__on_transfer_completed( char *filePath, int fileSize)
{
	fprintf(stderr, "[CMPL] %s of %dB\n", filePath, fileSize);
}


int main(int argc, char *argv[]){

		//Prepare structure for loading data from FILE/STDIN
	struct dataStruct data = {NULL, 0, 0};
	data.inputData = malloc(sizeof(char) * 1024);
	if(data.inputData == NULL){
		fprintf(stderr, "INTERNAL ERROR: Malloc function ERROR\n");
		exit(1);
	}
	memset(data.inputData, '\0', 1024);
	data.allocatedSpace = 1024;
	
	char *nameServer = NULL;
	char *dnsServer = NULL;
	int paramerProccessed = 1;

		//Process parameters of the program
	if(argc > 1 && !strcmp(argv[1], "-u")){
		//DNS server from the command line
		dnsServer = malloc(sizeof(char) * strlen(argv[2]));
		if(dnsServer == NULL){
			fprintf(stderr, "INTERNAL ERROR: Malloc function ERROR\n");
			exit(1);
		}
		memset(dnsServer,'\0' ,strlen(argv[2]));
		strcpy(dnsServer, argv[2]);
		// printf("equal: %s\n", dnsServer);

		paramerProccessed+=2;
	}
	else{
		//DNS server from the resolv.conf
		// printf("not-equal\n");

		FILE *file;
		char line[300];
		if((file = fopen("/etc/resolv.conf", "r")) == NULL){
			fprintf(stderr, "Error: Can't open /etc/resolv.conf file for DNS server \n");
			exit(1);
		}



		do{
			memset(line, '\0', 300);
			fgets(line, 300, file);

			nameServer = strtok(line, " ");
			dnsServer = strtok(0, " ");
			// printf("firstWrod: %s, IP: %s\n", nameServer, IPAddress);

		}while(line[0] == '#' || line[0] == ';' || strcmp(nameServer, "nameserver"));

	}

	if(argc < paramerProccessed + 2){
		fprintf(stderr, "Error: Wrong number of program arguments\n");
		exit(1);
	}
	char* BASE_HOST = argv[paramerProccessed];
	paramerProccessed++;
	char* DST_FILEPATH = argv[paramerProccessed];
	paramerProccessed++;
	// printf("%s, %s\n", BASE_HOST, DST_FILEPATH);
	bool readFromFILE = false;
	FILE* SRC_FILEPATH = NULL;

	if(argc != paramerProccessed){
		readFromFILE = true;
		if((SRC_FILEPATH = fopen(argv[paramerProccessed], "r")) == NULL){
			fprintf(stderr, "Error: Can't open input file\n");
			exit(1);
		}
		paramerProccessed++;
	}

	int character;
	if(!readFromFILE){
		while((character = getc(stdin)) != EOF){

			if(data.allocatedSpace == data.currentSpace){
				if((data.inputData = realloc(data.inputData, sizeof(char)*2*data.allocatedSpace)) == NULL){
					fprintf(stderr, "INTERNAL ERROR: Realloc error\n");
				}
				data.allocatedSpace*=2;
				// printf("REALLOC\n");
			}

			data.inputData[data.currentSpace] = character;
			data.currentSpace++;
		}
	}
	else{
		while((character = getc(SRC_FILEPATH)) != EOF){

			if(data.allocatedSpace == data.currentSpace){
				if((data.inputData = realloc(data.inputData, sizeof(char)*2*data.allocatedSpace)) == NULL){
					fprintf(stderr, "INTERNAL ERROR: Realloc error\n");
				}
				data.allocatedSpace*=2;
			}

			data.inputData[data.currentSpace] = character;
			data.currentSpace++;
		}

	}

	printf("data: %s\n", data.inputData);



	/**
	 * @link https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/
	*/
	struct sockaddr_in destination;
	int clientSocket;
	char buffer[1024];
	struct DNS_HEADER *dnsHeader = NULL;
	dnsHeader = (struct DNS_HEADER *)&buffer;

	if((clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= 0)
	{
		fprintf(stderr, "ERROR: Failed to create socket\n");
		exit(1);
	}


	destination.sin_family = AF_INET;
	destination.sin_port = htons(53);
	// printf("IPADDRESS: %s\n", dnsServer);
	destination.sin_addr.s_addr = inet_addr(dnsServer);
  

	dnsHeader->id = htons(1337);
	dnsHeader->qr = 0; 
	dnsHeader->opcode = 0; 
	dnsHeader->aa = 0; 
	dnsHeader->tc = 0; 
	dnsHeader->rd = 1; 
	dnsHeader->ra = 0; 
	dnsHeader->z = 0;
	dnsHeader->ad = 0;
	dnsHeader->cd = 0;
	dnsHeader->rcode = 0;
	dnsHeader->q_count = htons(1); //we have only 1 question
	// dnsHeader->q_count = 0; //we have only 1 question
	dnsHeader->ans_count = 0;
	dnsHeader->auth_count = 0;
	dnsHeader->add_count = 0;
	// if(sendto(clientSocket, ))

	char *qname = NULL;
	
	qname = malloc(sizeof(char) * 20);
	memset(qname, '\0', 20);
	strcpy(qname, "3www6google3com");

	struct QUESTION *qinfo = NULL;
	qname =(unsigned char*)&buffer[sizeof(struct DNS_HEADER)];

	qinfo =(struct QUESTION*)&buffer[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)];

	// qinfo->qtype = htons( query_type ); //type of the query , A , MX , CNAME , NS etc
	qinfo->qtype = htons(1); 
	qinfo->qclass = htons(1); //its internet (lol)


	if(sendto(clientSocket, (char*)buffer, sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION), 0, (struct sockaddr*)&destination, sizeof(destination)) < 0){

		fprintf(stderr, "Error; SENDTO failed");
		exit(1);
	}
	printf("SENDTO succed\n");

	return 0;
}