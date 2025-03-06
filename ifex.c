#include <stdio.h>
#include <net/if.h>

int main(void)
{
	char dev[IFNAMSIZ]="enp7s0";
	char buf[IF_NAMESIZE];
	unsigned int n;

	/* get index */
	n=if_nametoindex(dev);
	printf("if_index=%u, ",n);

	/* get name */
	if_indextoname(n, buf);
	printf("if_name=%s\n\n",buf);


	/*
	 * infi = [[if_nameindex], [if_nameindex]], [...,]
	 */
	struct if_nameindex *ifni;
	n=0;

	ifni=if_nameindex();
	for (;ifni->if_name;ifni++,n++)
		printf("if_index=%u, if_name=%s\n",ifni->if_index,ifni->if_name);
	ifni-=n;
	if_freenameindex(ifni);

	return 0;
}
