all: sender receiver


receiver: ./receiver/dns_receiver_events.c ./receiver/dns_receiver_events.h
	gcc ./receiver/dns_receiver_events.c -o DNSreceiver

sender: ./sender/dns_sender_events.c ./sender/dns_sender_events.h
	gcc ./sender/dns_sender_events.c -o DNSsender
