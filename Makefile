CC = rawnetcc
# CFLAGS = -I
# add $(CFLAGS) below behind compile commands

# We typically compile with:
# rawnetcc /tmp/arp_client eth.c ipv4.c ipv4_config.c ipv4_route_table.c arp_client.c arp.c

arp_client: arp_client.o
ipv4_client: ipv4_client.o

arp_client.o: arp.o
	$(CC) /tmp/arp_client.o arp_client.c
arp.o: eth.o ipv4.o ipv4_route_table.o ipv4_config.o
	$(CC) /tmp/arp.o arp.c
eth.o:
	$(CC) /tmp/eth.o eth.c
ipv4.o:
	$(CC) /tmp/ipv4.o ipv4.c
ipv4_route_table.o:
	$(CC) /tmp/ipv4_route_table.o ipv4_route_table.c
ipv4_config.c:
	$(CC) /tmp/ipv4_config.o ipv4_config.c
ipv4_client.c: eth.o ipv4.o ipv4_route_table.o ipv4_config.o
	$(CC) /tmp/ipv4_client.o ipv4_client.c

implement-arp_client: cp /.main arp_client \
				rm main

clean:
	rm main \
	rm arp_client \
	*.o
