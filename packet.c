#include <stdio.h>
#include <netpacket/packet.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void)
{
	struct sockaddr_ll sll;
	char dev[]="enp7s0";
	int fd;

	memset(&sll, 0, sizeof(sll));
	if ((fd=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))<0)
		return 0;
	sll.sll_ifindex=if_nametoindex(dev);


	close(fd);
	return 0;
}
