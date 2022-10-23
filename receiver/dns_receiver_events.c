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

/**
 *   @brief Function which convert domain na to DNS format
 *   @link https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/
 *   @author Silver Moon
*/
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


	if(argc != 3){
		fprintf(stderr, "Error: Wrong number of program arguments\n");
		exit(1);
	}
		//Process arguments of the program
	char* BASE_HOST = argv[1];
	unsigned char DST_DIRPATH[255]={'\0'};
	strcpy(DST_DIRPATH, argv[2]);


	int serverSocket;
	unsigned char buffer[512];
	unsigned char responseBuffer[512] ={'\0'};
	struct sockaddr_in serverAddr, clientAddr;


	if((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "ERROR: Failed to create socket\n");
		exit(1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr)); 
    memset(&clientAddr, 0, sizeof(clientAddr)); 

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(53);
	serverAddr.sin_addr.s_addr = INADDR_ANY;


		//Set options to socket
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
	// printf("Binding succesfull\n");

	int lenght = sizeof(clientAddr);
	int numOfBytesReceived = 0;
	char qname [253];
	memset(qname, '\0', 253);
	ChangetoDnsNameFormat(qname, BASE_HOST);

		//Process receiving data packets
	FILE *outputFile;
	int fileSize = 0;
	unsigned char data[253];
	unsigned char help1Data[253];
	unsigned char help2Data[253] ={'\0'};
	unsigned char decodedData[253] ={'\0'};
	struct DNS_HEADER *dnsResponseHeader = NULL;
	unsigned char *responseQname = NULL;
	struct QUESTION *responseQinfo = NULL;
	struct DNS_ANSWER *dnsResponseAnswer = NULL;

	while(true){
		memset(buffer, '\0', strlen(buffer));

		numOfBytesReceived = recvfrom(serverSocket, (unsigned char *)buffer, 512, MSG_WAITALL , (struct sockaddr *)&clientAddr, &lenght);
		if(numOfBytesReceived < 0){
			fprintf(stderr, "Error in recvfrom function. Didn.t receive data packet\n");
			exit(1);
		}

			//Extract client IP address
		unsigned char client_addr_str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(clientAddr.sin_addr), client_addr_str, INET_ADDRSTRLEN);
		printf("---------------------------\nReceived %d bytes from %s\n",  numOfBytesReceived, client_addr_str);


		struct DNS_HEADER *header = (struct DNS_HEADER *)&buffer;
		char *dns_query = (unsigned char*)&buffer[sizeof(struct DNS_HEADER)];
		
			
			//Check if dns_query coontains BASE_HOST
		char *index = strstr(dns_query, qname);
		if(index == NULL){
			continue;
		}
		
		int iteration = 0;
		memset(data, '\0', 253);
		memset(help1Data, '\0', 253);
		memset(help2Data, '\0', 253);
		memset(decodedData, '\0', 253);

			//Extract data from DNS_QUERY wtihout BASE_HOST
		while(&(dns_query[iteration]) != index){
			data[iteration] = dns_query[iteration];
			iteration++;
		}

			//Extract data from DNS format to normal normal string without hexa numbers before subdomain
		for(int i = 0; i < strlen(data); i++){
			strncpy(help1Data, &(data[i+1]), (int)data[i]);
			strcat(help2Data, help1Data);

			memset(help1Data, '\0', 253);
			i += (int)data[i];
		}

		memset(data, '\0', 253);
		strcpy(data, help2Data);

			//Decoding data from received packet		
		base32_decode(data, decodedData, 253);


		unsigned char DST_FILEPATH[255]={'\0'};
		unsigned char DST_DIRPATH_HELP[255]={'\0'};
		strcpy(DST_DIRPATH_HELP, DST_DIRPATH);

			//Prepare response
		memset(responseBuffer, '\0', strlen(responseBuffer));
			printf("EMPTY responebuff: %s\n", responseBuffer);
		dnsResponseHeader = (struct DNS_HEADER *)&responseBuffer;
		
		dnsResponseHeader->id = header->id;
		dnsResponseHeader->qr = htons(1); 
		dnsResponseHeader->opcode = 0; 
		dnsResponseHeader->aa = 0; 
		dnsResponseHeader->tc = 0; 
		dnsResponseHeader->rd = 1; 
		dnsResponseHeader->ra = 0; 
		dnsResponseHeader->z = 0;
		dnsResponseHeader->ad = 0;
		dnsResponseHeader->cd = 0;
		dnsResponseHeader->rcode = 0;
		dnsResponseHeader->q_count = htons(1); 
		dnsResponseHeader->ans_count = htons(1);
		dnsResponseHeader->auth_count = 0;
		dnsResponseHeader->add_count = 0;

		responseQname = (unsigned char*)&responseBuffer[sizeof(struct DNS_HEADER)];
		memset(responseQname, '\0', strlen(responseQname));
		strcat(responseQname, dns_query);
		responseQinfo = (struct QUESTION*)&responseBuffer[sizeof(struct DNS_HEADER) + (strlen((const char*)responseQname) + 1)];
		
		responseQinfo->qtype = htons(1); 
		responseQinfo->qclass = htons(1); 

		dnsResponseAnswer = (struct DNS_ANSWER*)&responseBuffer[sizeof(struct DNS_HEADER) + (strlen((const char*)responseQname) + 1) + sizeof(struct QUESTION)];

		dnsResponseAnswer->ans_type = htons(1);
		dnsResponseAnswer->name_offset = 0;
		dnsResponseAnswer->type = htons(1); 
		dnsResponseAnswer->qclass = htons(1);
		dnsResponseAnswer->ttl = 0;
		dnsResponseAnswer->rdlength = 0;
		dnsResponseAnswer->rdata = 0;


			//INIT PACKET receiver
		index = strstr(decodedData,"INITPATH[");
		if(index != NULL){


			int j = 0;
				//Extract file PATH from decodedData
			for(int i = 9; i < (strlen(decodedData)-1); i++){

				DST_FILEPATH[j] = decodedData[i];
				j++;
			}


				//Add '/' to the end of the PATH received as parameter of the receiver program
			if(DST_DIRPATH_HELP[strlen(DST_DIRPATH_HELP)-1] != '/'){
				DST_DIRPATH_HELP[strlen(DST_DIRPATH_HELP)] = '/';
			}
			strcat(DST_DIRPATH_HELP, DST_FILEPATH);


				//Create STRING which represent command for creating FOLDER PATH
			unsigned char DST_DIRPATH_COMMAND[265];
			sprintf(DST_DIRPATH_COMMAND, "mkdir -p %s", DST_DIRPATH_HELP); 


				//Removing file nam from the PATH
			int i = strlen(DST_DIRPATH_COMMAND)-1;
			for(i; i >= 0; i--){
				if(DST_DIRPATH_COMMAND[i] == '/'){
					break;
				}
			}

			for(i; i< strlen(DST_DIRPATH_COMMAND); i++){
				DST_DIRPATH_COMMAND[i] = '\0';
			}


				//Creating FOLDER PATH(contains only folders)
			system(DST_DIRPATH_COMMAND);


				//Open file for writing, if not created then create
			if((outputFile = fopen(DST_DIRPATH_HELP, "w")) == NULL){
				fprintf(stderr, "Error: Can't open output file \n");
				exit(1);
			}

			dns_receiver__on_query_parsed(DST_DIRPATH_HELP, data);

				//Send repsonse
			if(sendto(serverSocket, (unsigned char*)responseBuffer, sizeof(struct DNS_HEADER) + (strlen((const char*)responseQname)+1) + sizeof(struct QUESTION) + sizeof(struct DNS_ANSWER), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0){

				fprintf(stderr, "Error; SENDTO failed");
				exit(1);
			}
			dns_receiver__on_transfer_init(&(clientAddr.sin_addr));

			continue;
		}

		dns_receiver__on_query_parsed(DST_DIRPATH_HELP, data);

			//END PACKET received
		index = strstr(decodedData,"[ENDPACKET]");
		if(index != NULL){
			fclose(outputFile);
			dns_receiver__on_transfer_completed(DST_DIRPATH_HELP, fileSize);

				//Send repsonse
			if(sendto(serverSocket, (unsigned char*)responseBuffer, sizeof(struct DNS_HEADER) + (strlen((const char*)responseQname)+1) + sizeof(struct QUESTION) + sizeof(struct DNS_ANSWER), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0){

				fprintf(stderr, "Error; SENDTO failed");
				exit(1);
			}
			dns_receiver__on_transfer_init(&(clientAddr.sin_addr));

			continue;
		}

			//DATA PACKET received
		fileSize += strlen(decodedData);
		fprintf(outputFile,"%s", decodedData);
		dns_receiver__on_chunk_received( &(clientAddr.sin_addr),DST_DIRPATH_HELP, header->id, numOfBytesReceived);

			//Send repsonse
		if(sendto(serverSocket, (unsigned char*)responseBuffer, sizeof(struct DNS_HEADER) + (strlen((const char*)responseQname)+1) + sizeof(struct QUESTION) + sizeof(struct DNS_ANSWER), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0){

			fprintf(stderr, "Error; SENDTO failed");
			exit(1);
		}
		dns_receiver__on_transfer_init(&(clientAddr.sin_addr));
		
	}

	return 0;
}
