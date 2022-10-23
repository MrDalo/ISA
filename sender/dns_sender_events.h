#ifndef ISA22_DNS_SENDER_EVENTS_H
#define ISA22_DNS_SENDER_EVENTS_H

#include <netinet/in.h>

/**
 *   @brief DNS header structure
 *   @link https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/
*/
struct DNS_HEADER
{
	unsigned short id; // identification number

	unsigned char rd :1; // recursion desired
	unsigned char tc :1; // truncated message
	unsigned char aa :1; // authoritive answer
	unsigned char opcode :4; // purpose of message
	unsigned char qr :1; // query/response flag

	unsigned char rcode :4; // response code
	unsigned char cd :1; // checking disabled
	unsigned char ad :1; // authenticated data
	unsigned char z :1; // its z! reserved
	unsigned char ra :1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

/**
 *   @brief DNS header structure
 *   @link https://github.com/tbenbrahim/dns-tunneling-poc/blob/main/src/dns.h
 *   @license MIT
 *   @author Copyright (c) 2021 Tony BenBrahim
*/
struct DNS_ANSWER {
  uint8_t ans_type;
  uint8_t name_offset;
  uint16_t type;
  uint16_t qclass;
  uint32_t ttl;
  uint16_t rdlength;
  uint32_t rdata;
};


/**
 *   @brief Question structure
 *   @link https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/
*/
struct QUESTION
{
	unsigned short qtype;
	unsigned short qclass;
};



/**
 * @brief Structure for storing data from file/stdin
*/
struct dataStruct
{
	char* inputData;
	int allocatedSpace;
	int currentSpace;
};



/**
 * Tato metoda je volána klientem (odesílatelem) při zakódování části dat do doménového jména.
 * V případě použití více doménových jmen pro zakódování dat, volejte funkci pro každé z nich.
 *
 * @param filePath Cesta k cílovému souboru
 * @param chunkId Identifikátor části dat
 * @param encodedData Zakódovaná data do doménového jména (např.: "acfe2a42b.example.com")
 */
void dns_sender__on_chunk_encoded(char *filePath, int chunkId, char *encodedData);

/**
 * Tato metoda je volána klientem (odesílatelem) při odeslání části dat serveru (příjemci).
 *
 * @param dest IPv4 adresa příjemce
 * @param filePath Cesta k cílovému souboru (relativní na straně příjemce)
 * @param chunkId Identifikátor části dat
 * @param chunkSize Velikost části dat v bytech
 */
void dns_sender__on_chunk_sent(struct in_addr *dest, char *filePath, int chunkId, int chunkSize);

/**
 * Tato metoda je volána klientem (odesílatelem) při odeslání části dat serveru (příjemci).
 *
 * @param dest IPv6 adresa příjemce
 * @param filePath Cesta k cílovému souboru (relativní na straně příjemce)
 * @param chunkId Identifikátor části dat
 * @param chunkSize Velikost části dat v bytech
 */
void dns_sender__on_chunk_sent6(struct in6_addr *dest, char *filePath, int chunkId, int chunkSize);

/**
 * Tato metoda je volána klientem (odesílatelem) při zahájení přenosu serveru (příjemci).
 *
 * @param dest IPv4 adresa příjemce
 */
void dns_sender__on_transfer_init(struct in_addr *dest);

/**
 * Tato metoda je volána klientem (odesílatelem) při zahájení přenosu serveru (příjemci).
 *
 * @param dest IPv6 adresa příjemce
 */
void dns_sender__on_transfer_init6(struct in6_addr *dest);

/**
 * Tato metoda je volána klientem (odesílatelem) při dokončení přenosu jednoho souboru serveru (příjemci).
 *
 * @param filePath Cesta k cílovému souboru
 * @param fileSize Celková velikost přijatého souboru v bytech
 */
void dns_sender__on_transfer_completed( char *filePath, int fileSize);

#endif //ISA22_DNS_SENDER_EVENTS_H
