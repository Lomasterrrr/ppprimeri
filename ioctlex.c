#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>

int main(void)
{
	struct ifreq ifr={0}; /* init!!! */
	int fd;

	snprintf(ifr.ifr_name,IFNAMSIZ,"enp7s0");

	fd=socket(AF_INET,SOCK_DGRAM,0);

	/* src mac */
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
		(unsigned char)ifr.ifr_hwaddr.sa_data[0],
		(unsigned char)ifr.ifr_hwaddr.sa_data[1],
		(unsigned char)ifr.ifr_hwaddr.sa_data[2],
		(unsigned char)ifr.ifr_hwaddr.sa_data[3],
		(unsigned char)ifr.ifr_hwaddr.sa_data[4],
		(unsigned char)ifr.ifr_hwaddr.sa_data[5]);

	/* src ipv4 */
	ioctl(fd, SIOCGIFADDR, &ifr);
	printf("%hhu.%hhu.%hhu.%hhu\n",
		(unsigned char)ifr.ifr_addr.sa_data[0+2],
		(unsigned char)ifr.ifr_addr.sa_data[1+2],
		(unsigned char)ifr.ifr_addr.sa_data[2+2],
		(unsigned char)ifr.ifr_addr.sa_data[3+2]);

	/* mtu */
	ioctl(fd, SIOCGIFMTU, &ifr);
	printf("%d\n",ifr.ifr_mtu);

	/* flags */
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	printf("%s %s %s\n",
		(ifr.ifr_flags&IFF_UP)?"UP":"",
		(ifr.ifr_flags&IFF_BROADCAST)?"BROADCAST":"",
		(ifr.ifr_flags/* & */&IFF_LOOPBACK)?"LOOPBACK":"");

	close(fd);
	return 0;
}
