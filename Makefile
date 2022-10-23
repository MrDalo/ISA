CFLAGS = -std=c99
.PHONY = clean all receiver sender



all: receiver sender clean


receiver: base32.o dns_receiver_events.o
	gcc $(CFLAGS) $^ -o dns_sender

sender: base32.o dns_sender_events.o
	gcc $(CFLAGS) $^ -o dns_receiver


# %.o: ./sender/dns_sender_events.c ./receiver/dns_receiver_events.c %.c
# 	gcc $(CFLAGS) -c $^ -o $@

dns_receiver_events.o: ./receiver/dns_receiver_events.c
	gcc $(CFLAGS) -c $^ -o dns_receiver_events.o

dns_sender_events.o: ./sender/dns_sender_events.c
	gcc $(CFLAGS) -c $^ -o dns_sender_events.o

base32.o: base32.c
	gcc $(CFLAGS) -c $^ -o base32.o

clean: 
	rm *.o