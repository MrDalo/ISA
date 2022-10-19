#ifndef ISA22_DNS_RECEIVER_EVENTS_H
#define ISA22_DNS_RECEIVER_EVENTS_H

#include <netinet/in.h>

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
 * Tato metoda je volána serverem (příjemcem) při přijetí zakódovaných dat od klienta (odesílatele).
 * V případě použití více doménových jmen pro zakódování dat, volejte funkci pro každé z nich.
 *
 * @param filePath Cesta k cílovému souboru
 * @param encodedData Zakódovaná data do doménového jména (např.: "acfe2a42b.example.com")
 */
void dns_receiver__on_query_parsed(char *filePath, char *encodedData);

/**
 * Tato metoda je volána serverem (příjemcem) při příjmu části dat od klienta (odesílatele).
 *
 * @param source IPv4 adresa odesílatele
 * @param filePath Cesta k cílovému souboru
 * @param chunkId Identifikátor části dat
 * @param chunkSize Velikost části dat v bytech
 */
void dns_receiver__on_chunk_received(struct in_addr *source, char *filePath, int chunkId, int chunkSize);

/**
 * Tato metoda je volána serverem (příjemcem) při příjmu části dat od klienta (odesílatele).
 *
 * @param source IPv6 adresa odesílatele
 * @param filePath Cesta k cílovému souboru
 * @param chunkId Identifikátor části dat
 * @param chunkSize Velikost části dat v bytech
 */
void dns_receiver__on_chunk_received6(struct in6_addr *source, char *filePath, int chunkId, int chunkSize);

/**
 * Tato metoda je volána serverem (příjemcem) při zahájení přenosu od klienta (odesílatele).
 *
 * @param source IPv4 adresa odesílatele
 */
void dns_receiver__on_transfer_init(struct in_addr *source);

/**
 * Tato metoda je volána serverem (příjemcem) při zahájení přenosu od klienta (odesílatele).
 *
 * @param source IPv6 adresa odesílatele
 */
void dns_receiver__on_transfer_init6(struct in6_addr *source);

/**
 * Tato metoda je volána serverem (příjemcem) při dokončení přenosu jednoho souboru od klienta (odesílatele).
 *
 * @param filePath Cesta k cílovému souboru
 * @param fileSize Celková velikost přijatého souboru v bytech
 */
void dns_receiver__on_transfer_completed(char *filePath, int fileSize);

#endif //ISA22_DNS_RECEIVER_EVENTS_H
