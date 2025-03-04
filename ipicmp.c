#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
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

/* https://github.com/openbsd/src/blob/master/sys/lib/libsa/in_cksum.c */
static inline u_int16_t in_cksum(const void *p, size_t l)
{
	unsigned int sum = 0;
	int len;
	const u_char *cp = p;

	if (l >= (1 << 16))
		return 0;
	len = (int)l;

	if (((long)cp & 1) == 0) {
		while (len > 1) {
			sum += htons(*(const u_short *)cp);
			cp += 2;
			len -= 2;
		}
	} else {
		while (len > 1) {
			sum += *cp++ << 8;
			sum += *cp++;
			len -= 2;
		}
	}
	if (len == 1)
		sum += *cp << 8;

	sum = (sum >> 16) + (sum & 0xffff);
	sum += sum >> 16;
	sum = ntohs(sum);
	return (0xffff & ~sum);
}

static u8 *traceprobe(int ttl, u8 *srcmac, struct in_addr *dst,
	struct in_addr *src, u32 *outlen)
{
	struct ethhdr *eth;
	struct iphdr *ip;
	struct icmp *icmp;
	u8 *probe;

	*outlen=ETH_HLEN+sizeof(struct iphdr)+sizeof(struct icmp);
	if (!(probe=calloc(1,*outlen)))
		return NULL;

	eth=(struct ethhdr*)probe;
	/* my dst */
	eth->h_dest[0]=0x04;
	eth->h_dest[1]=0xbf;
	eth->h_dest[2]=0x6d;
	eth->h_dest[3]=0x0d;
	eth->h_dest[4]=0x3a;
	eth->h_dest[5]=0x50;
	memcpy(eth->h_source,srcmac,ETH_ALEN);
	eth->h_proto=htons(ETHERTYPE_IP);

	ip=(struct iphdr*)(probe+14);
	ip->protocol=IPPROTO_ICMP;
	ip->ttl=ttl;
	ip->version=4;
	ip->ihl=5;
	ip->id=htons(7777);
	ip->tos=0;
	ip->tot_len=htons(*outlen-14);
	ip->frag_off=htons(IP_DF);
	ip->saddr=src->s_addr;
	ip->daddr=dst->s_addr;
	ip->check=in_cksum((u16*)ip, sizeof(struct iphdr));

	icmp=(struct icmp*)(probe+14+20);
	icmp->icmp_code=0;
	icmp->icmp_type=ICMP_ECHO;
	icmp->icmp_id=htons(777);
	icmp->icmp_seq=htons(1);
	icmp->icmp_cksum=in_cksum((u16*)icmp, sizeof(struct icmp));

	return probe;
}

static inline const char *getdns(struct in_addr dst)
{
	static char res[2048+2];
	struct sockaddr_in sa;
	char dnsbuf[2048];

	memset(dnsbuf, 0, sizeof(dnsbuf));
	memset(&sa, 0, sizeof(sa));
	sa.sin_family=AF_INET;
	sa.sin_addr.s_addr=dst.s_addr;

	if (getnameinfo((struct sockaddr*)&sa,
		sizeof(sa), dnsbuf, sizeof(dnsbuf),
		NULL, 0, 0)==0) {
		snprintf(res, sizeof(res), "(%s)", dnsbuf);
		return res;
	}

	return "(\?\?\?)";
}

static inline double time(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec+t.tv_usec/1e6;
}

int main(int argc, char **argv)
{
	struct ether_addr	src;
	struct in_addr		srcip4;
	int			id;
	struct sockaddr_ll	sll={0};
	int			fd,n;
	u8			*icmp;
	struct in_addr		dst, outip;
	u32			outlen;
	char			recvbuf[1024];
	double			s;
	u8			aeflag,fuckaeeflag;

	getupif(&id, &src, &srcip4);

	if ((fd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))<0)
		return 0;

	sll.sll_ifindex=id;
	dst.s_addr=inet_addr(argv[1]); /* target */

	for (n=1;n<=30;n++) {
		icmp=traceprobe(n, src.ether_addr_octet, &dst, &srcip4, &outlen);
		
		sendto(fd, icmp, outlen, 0, (struct sockaddr *)&sll, sizeof(sll));
		s=time();
		aeflag=0,fuckaeeflag=0;
		for (;;) {
			struct ethhdr *datalink;
			struct icmp *icmphdr;
			struct iphdr *iphdr, *iphdr_2;

			if ((time()-s)>=1)
				break;

			if (recv(fd, recvbuf, sizeof(recvbuf), 0)<0)
				return 0;
			datalink=(struct ethhdr*)recvbuf;
			if (ntohs(datalink->h_proto)!=ETHERTYPE_IP)
				continue;
			iphdr=(struct iphdr*)(recvbuf+14);
			if (iphdr->saddr==dst.s_addr) {
				fuckaeeflag++;
				aeflag++;
				outip.s_addr=iphdr->saddr;
				break;
			}
			if (iphdr->protocol!=IPPROTO_ICMP)
				continue;
			icmphdr=(struct icmp*)(recvbuf+14+20);
			if (icmphdr->icmp_type!=ICMP_TIMXCEED)
				continue;
			if (iphdr->daddr!=srcip4.s_addr)
				continue;
			outip.s_addr=iphdr->saddr;
			aeflag++;
			break;
		}

		free(icmp);
		if (aeflag) 
			printf("%s %s\n",inet_ntoa(outip), getdns(outip));
		else
			puts("???");
		if (fuckaeeflag) {
			printf("DOSTIGNUTO FOR %d TAKTOV\n", n);
			break;
		}
	}

	close(fd);
	return 0;
}
