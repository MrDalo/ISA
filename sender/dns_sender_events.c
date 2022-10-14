#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "dns_sender_events.h"
#include <string.h>
#include <stdlib.h>

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

	char *dnsServer = NULL;

	if(argc > 1 && !strcmp(argv[1], "-u")){
		//DNS server from the command line
		dnsServer = malloc(sizeof(char) * strlen(argv[2]));
		memset(dnsServer,'\0' ,strlen(argv[2]));
		strcpy(dnsServer, argv[2]);
		printf("equal: %s\n", dnsServer);

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


		char *nameServer = NULL;
		char *IPAddress = NULL;

		do{
			memset(line, '\0', 300);
			fgets(line, 300, file);

			nameServer = strtok(line, " ");
			IPAddress = strtok(0, " ");
			printf("firstWrod: %s, IP: %s\n", nameServer, IPAddress);

		}while(line[0] == '#' || line[0] == ';' || strcmp(nameServer, "nameserver"));

	}
	return 0;
}