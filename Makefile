CFLAGS = -std=c99 -Wall 
.PHONY = clean all receiver sender



all: receiver sender clean


receiver: base32.o dns_receiver_events.o
	gcc $(CFLAGS) base32.o dns_receiver_events.o -o dns_receiver

sender: base32.o dns_sender_events.o
	gcc $(CFLAGS) base32.o dns_sender_events.o -o dns_sender


# %.o: ./sender/dns_sender_events.c ./receiver/dns_receiver_events.c %.c
# 	gcc $(CFLAGS) -c $^ -o $@

dns_receiver_events.o: ./receiver/dns_receiver_events.c
	gcc $(CFLAGS) -c ./receiver/dns_receiver_events.c -o dns_receiver_events.o

dns_sender_events.o: ./sender/dns_sender_events.c
	gcc $(CFLAGS) -c ./sender/dns_sender_events.c -o dns_sender_events.o

base32.o: base32.c
	gcc $(CFLAGS) -c base32.c -o base32.o

clean: 
	rm *.o
