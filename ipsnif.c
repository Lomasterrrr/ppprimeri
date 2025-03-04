#include <net/ethernet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <netpacket/packet.h>

static inline double time(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec+t.tv_usec/1e6;
}

static inline const char *util_bytesconv(size_t bytes)
{
	const char *sizes[]={
		"B", "KiB", "MiB", "GiB", "TiB",
		"PiB", "EiB"
	};
	static char buffer[32];
	double c=(double)bytes;
	int i=0;

	while (c>=1024&&i<6) { c/=1024; i++; }
	snprintf(buffer, sizeof(buffer), "%.2f %s", c, sizes[i]);
	return buffer;
}

int main(int argc, char **argv)
{
	size_t		tcp,udp,icmp,sctp,igmp,tot;
	char		buf[ETHER_MAX_LEN];
	struct iphdr	*ip=NULL;
	struct ethhdr	*eth=NULL;
	ssize_t		ret;
	double		s;
	int		fd,wait;

	wait=atoi(argv[1]);
	tcp=udp=icmp=sctp=igmp=tot=0;
	fd=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	s=time();

	for (;;) {
		ret=recv(fd, buf, sizeof(buf), 0);
		if (ret<0)
			break;
		eth=(struct ethhdr*)(buf);
		if (ntohs(eth->h_proto)!=ETHERTYPE_IP)
			continue;
		tot+=ret;
		ip=(struct iphdr*)(buf+ETH_HLEN);
		switch (ip->protocol) {
			case IPPROTO_TCP: tcp++; break;
			case IPPROTO_UDP: udp++; break;
			case IPPROTO_ICMP: icmp++; break;
			case IPPROTO_IGMP: igmp++; break;
			case IPPROTO_SCTP: sctp++; break;
		}
		if ((time())-s>=wait)
			break;
	}

	printf("received %s bytes: tcp=%ld udp=%ld icmp=%ld sctp=%ld igmp=%ld\n",
		util_bytesconv(tot),tcp,udp,icmp,sctp,igmp);

	close(fd);
	return 0;
}
