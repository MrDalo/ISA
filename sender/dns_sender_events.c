#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "dns_sender_events.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "../base32.h"
#include <math.h>

#define NETADDR_STRLEN (INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN)
#define CREATE_IPV4STR(dst, src) char dst[NETADDR_STRLEN]; inet_ntop(AF_INET, src, dst, NETADDR_STRLEN)
#define CREATE_IPV6STR(dst, src) char dst[NETADDR_STRLEN]; inet_ntop(AF_INET6, src, dst, NETADDR_STRLEN)
#define BASE32_LENGTH_ENCODE(src_size) (((src_size)*8 + 4) / 5)
#define BASE32_LENGTH_DECODE(src_size) (ceil((src_size)) / 1.6)


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


void ChangetoDnsNameFormat(char* dns, char* host) 
{
	int lock = 0 , i;
	strcat((char*)host,".");
	char help[5];
	unsigned char *ptr = NULL;
	
	for(i = 0 ; i < strlen((char*)host) ; i++) 
	{

		if(host[i]=='.') 
		{
			// sprintf(help, "%x", i-lock);
			// strcat((char*)dns, help);
			*dns++=(unsigned char)(i-lock);
			for(;lock<i;lock++) 
			{
				*dns++=host[lock];
			}
			lock++;
		}
	}
	*dns++=(unsigned char)(0);
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
			// printf("firstWrod: %s, IP: %s\n", nameServer, dnsServer);

		}while(line[0] == '#' || line[0] == ';' || strcmp(nameServer, "nameserver"));

	}
	

	if(argc < paramerProccessed + 2){
		fprintf(stderr, "Error: Wrong number of program arguments\n");
		exit(1);
	}
	char* BASE_HOST = argv[paramerProccessed];
	paramerProccessed++;
	char DST_FILEPATH[strlen(argv[paramerProccessed])];
	strcpy(DST_FILEPATH, argv[paramerProccessed]);

	paramerProccessed++;
	printf("1BASE_HOST: %s, DST_FILEPATH: %s\n", BASE_HOST, DST_FILEPATH);
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

	
	// printf("data: %s\n", data.inputData);
	printf("2BASE_HOST: %s, DST_FILEPATH: %s\n", BASE_HOST, DST_FILEPATH);


	/**
	 * @link https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/
	*/
	
	struct sockaddr_in destination;
	int clientSocket;

	if((clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= 0)
	{
		fprintf(stderr, "ERROR: Failed to create socket\n");
		exit(1);
	}

	destination.sin_family = AF_INET;
	destination.sin_port = htons(53);
	destination.sin_addr.s_addr = inet_addr(dnsServer);

	dns_sender__on_transfer_init(&(destination.sin_addr));
	
		//INIT PACKET
	unsigned char buffer[512];
	memset(buffer,'\0', 512);
	struct DNS_HEADER *dnsHeader = NULL;
	dnsHeader = (struct DNS_HEADER *)&buffer;

	dnsHeader->id = (unsigned short) htons(getpid());
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
	dnsHeader->q_count = htons(1); 
	dnsHeader->ans_count = 0;
	dnsHeader->auth_count = 0;
	dnsHeader->add_count = 0;

	unsigned char *qname = (unsigned char*)&buffer[sizeof(struct DNS_HEADER)];
	unsigned char baseHostForQname[253] ={'\0'};


	printf("3BASE_HOST: %s, DST_FILEPATH: %s\n", BASE_HOST, DST_FILEPATH);
	ChangetoDnsNameFormat(baseHostForQname , BASE_HOST);
	printf("4BASE_HOST: %s, DST_FILEPATH: %s\n", BASE_HOST, DST_FILEPATH);
	
	// printf("arrayForQname: %s\n", baseHostForQname);

	// int neededDataLength = BASE32_LENGTH_DECODE(253-strlen(baseHostForQname));
	// printf("neededDataLenght: %d\n", neededDataLength);
	// int numberOfWritenChars = base32_encode((uint8_t *)data.inputData, neededDataLength, (uint8_t *)base32_data_buf, 256);

	// strcat(qname, base32_data_buf);
	// strcat(qname, baseHostForQname);	
	// printf("QNAME: %s, numbeOfWritenChars: %d\n", qname, numberOfWritenChars);

	// sprintf(qname, "INIT PACKET PATH[%s]", DST_FILEPATH);
	strcat(qname, baseHostForQname);

	struct QUESTION *qinfo = NULL;
	qinfo =(struct QUESTION*)&buffer[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)];

	qinfo->qtype = htons(1); 
	qinfo->qclass = htons(1); 
	
	// strcat(buffer, data.inputData);
	// printf("sizeof: %ld\n", sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION));
	// printf("buffer3: %s\n", buffer);
	if(sendto(clientSocket, (unsigned char*)buffer, sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION), 0, (struct sockaddr*)&destination, sizeof(destination)) < 0){

		fprintf(stderr, "Error; SENDTO failed");
		exit(1);
	}
	// printf("SENDTO succed\n");

		//DATA PACKET
	memset(buffer,'\0', 512);
	dnsHeader = (struct DNS_HEADER *)&buffer;

	dnsHeader->id = (unsigned short) htons(getpid());

	unsigned char base32_data_buf[253] = {'\0'};

	int neededDataLength = BASE32_LENGTH_DECODE(253-strlen(baseHostForQname));
	int numberOfWritenChars = base32_encode((uint8_t *)data.inputData, neededDataLength, (uint8_t *)base32_data_buf, 253);
	
	dns_sender__on_chunk_encoded(DST_FILEPATH, dnsHeader->id, base32_data_buf);
	
	strcat(qname, base32_data_buf);
	strcat(qname, baseHostForQname);

	if(sendto(clientSocket, (unsigned char*)buffer, sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION), 0, (struct sockaddr*)&destination, sizeof(destination)) < 0){

		fprintf(stderr, "Error; SENDTO failed");
		exit(1);
	}

	dns_sender__on_chunk_sent(&(destination.sin_addr) ,DST_FILEPATH, dnsHeader->id, strlen(buffer)*sizeof(char));

		//FINAL PACKET
	memset(buffer,'\0', 512);
	dnsHeader->id = (unsigned short) htons(getpid());
	strcat(qname, "[END PACKET]");
	strcat(qname, baseHostForQname);

	if(sendto(clientSocket, (unsigned char*)buffer, sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION), 0, (struct sockaddr*)&destination, sizeof(destination)) < 0){

		fprintf(stderr, "Error; SENDTO failed");
		exit(1);
	}

	dns_sender__on_transfer_completed(DST_FILEPATH, strlen(data.inputData) * sizeof(char));

	
	return 0;
}