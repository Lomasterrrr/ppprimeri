#include <stdio.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;

static inline void frame0(struct ether_header *eth, u8 *src, u8 *dst, u16 type)
{
	memcpy(eth->ether_dhost, dst, 6);
	memcpy(eth->ether_shost, src, 6);
	eth->ether_type=htons(type);
}

static inline void frame1(struct ethhdr *eth, u8 *src, u8 *dst, u16 type)
{
	memcpy(eth->h_dest, dst, 6);
	memcpy(eth->h_source, src, 6);
	eth->h_proto=htons(type);
}

int main(void)
{
	struct ether_addr *src, *dst;
	struct ether_header	eth0;
	struct ethhdr		eth1;

	src=ether_aton("40:b0:76:47:8f:9a");
	dst=ether_aton("04:bf:6d:0d:3a:50");

	frame0(&eth0, src->ether_addr_octet, dst->ether_addr_octet, ETHERTYPE_IP);
	frame1(&eth1, src->ether_addr_octet, dst->ether_addr_octet, ETHERTYPE_IP);

	return 0;
}
