arp_client: eth.o ipv4.o arp.o

eth.o:
	rawnetcc /tmp/eth.o eth/eth.c
ipv4.o:
	rawnetcc /tmp/ipv4.o ipv4/ipv4.c
arp.o:
	rawnetcc /tmp/arp.o arp/arp.c

implement-arp_client: cp /.main arp_client \
				rm main

clean:
	rm main \
	*.o
