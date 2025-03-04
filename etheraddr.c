#include <netinet/ether.h>
#include <stdio.h>

static inline void macputs(struct ether_addr *m)
{
	int n;
	for (n=0;n<ETHER_ADDR_LEN;n++)
		printf("%02x%s", m->ether_addr_octet[n],
				(n==5)?"\n":":");
}

int main(void)
{
	struct ether_addr *mac;
	char *p;

	/* ascii to mac */
	mac=ether_aton("40:b0:76:47:8f:9a");
	macputs(mac);

	ether_aton_r("40:b0:76:47:8f:9a", mac);
	macputs(mac);

	/* mac to ascii */
	p=ether_ntoa(mac);
	printf("%s\n",p);

	ether_ntoa_r(mac,p);
	printf("%s\n",p);


	/* get host to mac /etc/ethers */
	char hostname[256];
	int ret;

	mac=ether_aton("02:42:ac:11:00:02");
	ret=ether_ntohost(hostname, mac);
	if (!ret)
		printf("%s\n",hostname);


	return 0;
}
