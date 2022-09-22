main: arp_client.o

arp_client.o: arp.o
	rawnetcc /tmp/arp.o arp/arp_client.c
arp.o: eth.o ipv4.o
	rawnetcc /tmp/arp.o arp/arp.c
eth.o:
	rawnetcc /tmp/eth.o eth/eth.c
ipv4.o:
	rawnetcc /tmp/ipv4.o ipv4/ipv4.c

implement-arp_client: cp /.main arp_client \
				rm main

clean:
	rm main \
	rm arp_client \
	*.o
