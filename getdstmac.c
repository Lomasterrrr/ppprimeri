#include <linux/if_ether.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/types.h> /* u8, u16, u32 */

typedef __u8 u8;
typedef __u16 u16;
typedef __u32 u32;

static u8 *c_arpframe(u8 *srcmac, struct in_addr *srcip4, struct in_addr *to, u32 *outlen)
{
	struct ether_arp *arp;
	struct ethhdr *eth;
	u8 *frame;

	*outlen=ETH_HLEN+sizeof(struct ether_arp);
	if (!(frame=calloc(1,*outlen)))
		return NULL;

	eth=(struct ethhdr*)frame;
	memset(eth->h_dest,0xff,ETH_ALEN);	/* broadcast */
	memcpy(eth->h_source,srcmac,ETH_ALEN);
	eth->h_proto=htons(ETHERTYPE_ARP);

	arp=(struct ether_arp*)(frame+ETH_HLEN);
	arp->arp_op=htons(ARPOP_REQUEST);
	arp->arp_hrd=htons(ARPHRD_ETHER);
	arp->arp_hln=ETH_ALEN;
	arp->arp_pln=4;
	arp->arp_pro=htons(ETHERTYPE_IP);

	memcpy(arp->arp_sha,srcmac,ETH_ALEN);
	memset(arp->arp_tha,0x00,ETH_ALEN);
	memcpy(arp->arp_spa,&srcip4->s_addr,4);
	memcpy(arp->arp_tpa,&to->s_addr,4);

	return frame;
}

static void getupif(int *index, struct ether_addr *src, struct in_addr *srcip4)
{
	struct if_nameindex *ifni;
	struct ifreq ifr={0};
	int n,fd;
	
	*index=-1,n=0;

	if ((fd=socket(AF_INET,SOCK_DGRAM,0))<0)
		return;
	ifni=if_nameindex();
	for (;ifni->if_name;ifni++,n++) {
		snprintf(ifr.ifr_name,IFNAMSIZ,"%s",ifni->if_name);
		if ((ioctl(fd, SIOCGIFFLAGS, &ifr))<0)
			goto exit;
		if (ifr.ifr_flags&IFF_UP&&
			!(ifr.ifr_flags&IFF_LOOPBACK)&&
			!(ifr.ifr_flags&IFF_POINTOPOINT)) {
			if ((ioctl(fd, SIOCGIFHWADDR, &ifr))<0)
				goto exit;
			memcpy(src->ether_addr_octet,
				ifr.ifr_ifru.ifru_hwaddr.sa_data, 6);
			if ((ioctl(fd, SIOCGIFADDR, &ifr))<0)
				goto exit;
			memcpy(&srcip4->s_addr, (ifr.ifr_addr.sa_data+2), 4);
			*index=ifni->if_index;
			goto exit;
		}
	}
exit:
	ifni-=n;
	if_freenameindex(ifni);
	close(fd);
}

int main(int argc, char **argv)
{
	/* get index up interface */
	struct ether_addr	src;
	struct in_addr		srcip4;
	int			id;

	getupif(&id, &src, &srcip4);


	/* send arp */
	struct sockaddr_ll	sll={0};
	int			fd;
	u8			*arp;
	struct in_addr		dst;
	u32			outlen;

	if ((fd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))<0)
		return 0;
	sll.sll_ifindex=id;

	dst.s_addr=inet_addr(argv[1]); /* target */
	arp=c_arpframe(src.ether_addr_octet, &srcip4, &dst, &outlen);
	sendto(fd, arp, outlen, 0, (struct sockaddr *)&sll, sizeof(sll));


	/* recv arp */
	char			recvbuf[2024];
	struct			ether_arp *hdr, *hdr1;
	int			n;

	L1:
	n=recv(fd, recvbuf, sizeof(recvbuf), 0);
	hdr=(struct ether_arp*)(recvbuf+ETH_HLEN);
	hdr1=(struct ether_arp*)(arp+ETH_HLEN);

	if (hdr->arp_spa[0]==hdr1->arp_tpa[0]&&
		hdr->arp_spa[1]==hdr1->arp_tpa[1]&&
		hdr->arp_spa[2]==hdr1->arp_tpa[2]&&
		hdr->arp_spa[3]==hdr1->arp_tpa[3])
		goto exit;
	goto L1;


	/* print addr */
exit:
	for (n=0;n<6;n++)
		printf("%02x%s",hdr->arp_sha[n], (n==5)?"\n":":");

	free(arp);
	close(fd);
		
	return 0;
}
