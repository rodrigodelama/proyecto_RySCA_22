CC = rawnetcc
CFLAGS = -I

main: arp_client.o

arp_client.o: arp.o
	$(CC) /tmp/arp.o arp/arp_client.c $(CFLAGS)
arp.o: eth.o ipv4.o
	$(CC) /tmp/arp.o arp/arp.c $(CFLAGS)
eth.o:
	$(CC) /tmp/eth.o eth/eth.c $(CFLAGS)
ipv4.o:
	$(CC) /tmp/ipv4.o ipv4/ipv4.c $(CFLAGS)

implement-arp_client: cp /.main arp_client \
				rm main

clean:
	rm main \
	rm arp_client \
	*.o
