#include <netinet/if_ether.h>
#include <stdio.h>
#include <netpacket/packet.h>
#include <netinet/ether.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

int main(void)
{
	
	/*
	 * 0000   ff ff ff ff ff ff 40 b0 76 47 8f 9a 08 06 00 01
	 * 0010   08 00 06 04 00 01 40 b0 76 47 8f 9a c0 a8 01 21
	 * 0020   ff ff ff ff ff ff c0 a8 01 01
	 */
	unsigned char etharp[42] = {0x4, 0xbf, 0x6d, 0xd, 0x3a, 0x50, 0x40, 0xb0,
				0x76, 0x47, 0x8f, 0x9a, 0x8, 0x6, 0x0, 0x1, 0x8,
				0x0, 0x6, 0x4, 0x0, 0x1, 0x40, 0xb0, 0x76, 0x47,
				0x8f, 0x9a, 0xc0, 0xa8, 0x1, 0x21, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xc0, 0xa8, 0x1, 0x1};
	unsigned char bytes[] = {0x40, 0xb0, 0x76, 0x47, 0x8f, 0x9a};

	struct sockaddr_ll sll={0}; /* init !!! */
	char recvbuf[1024];
	int fd,n;

	/* socket, sendto, recv/recvfrom, close */
	sll.sll_ifindex=2;
	sll.sll_hatype=0;
	sll.sll_family=AF_PACKET;
	sll.sll_protocol=ETH_P_ARP;
	sll.sll_pkttype=PACKET_OTHERHOST;
	sll.sll_halen=6;
	memcpy(sll.sll_addr, etharp, 6);


	fd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	if (bind(fd, (struct sockaddr *)&sll, sizeof(sll))<0)
		return 0;
	send(fd, etharp, sizeof(etharp), 0);

	//sendto(fd, etharp, sizeof(etharp), 0, (struct sockaddr *)&sll, sizeof(sll));
	for (;;) {
		recv(fd, recvbuf, sizeof(recvbuf), 0);
		perror("");
	}

	close(fd);

	/*
	*/

	return 0;
}

