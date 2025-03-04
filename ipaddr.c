#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void ip4addr(void)
{
	char ipbuf[INET_ADDRSTRLEN];
	struct in_addr ip4;

	/* broadcast */
	ip4.s_addr=INADDR_BROADCAST; /* или for example INADDR_LOOPBACK */
	printf("%s\n", inet_ntoa(ip4));

	/* ascii ip4 to in_addr (not htonl) */
	ip4.s_addr=inet_addr("64.233.162.113");
	printf("%s\n", inet_ntoa(ip4));

	/* ascii ip4 to in_addr (with htonl) */
	ip4.s_addr=inet_network("64.233.162.113");
	printf("%s\n", inet_ntoa(ip4));

	/*
	 * inet_nework("") == htonl(inet_addr(""))
	 */



	/* to in_addr */
	inet_pton(AF_INET, "64.233.162.113", &ip4);

	/* to ascii */
	inet_ntop(AF_INET, &ip4, ipbuf, INET_ADDRSTRLEN);
	printf("%s\n",ipbuf);

}

void ip6addr(void)
{
	char ipbuf[INET6_ADDRSTRLEN];
	struct in6_addr ip6;

	/* to in6_addr */
	inet_pton(AF_INET6, "2001:20::", &ip6);

	/* to ascii */
	inet_ntop(AF_INET6, &ip6, ipbuf, INET6_ADDRSTRLEN);
	printf("%s\n",ipbuf);


	/* loop back */
	struct in6_addr ip6_1=IN6ADDR_LOOPBACK_INIT; /* константа которая only при init*/

	inet_ntop(AF_INET6, &ip6_1, ipbuf, INET_ADDRSTRLEN);
	printf("%s\n",ipbuf);
}


int main(void)
{
	ip4addr();
	ip6addr();
	return 0;
}
