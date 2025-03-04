#include <stdio.h>
#include <netinet/tcp.h>

int main(void)
{
	struct tcphdr a,b;

	/* set flags a */
	a.syn=1, a.ack=1;
	if (a.syn) putchar('S');

	/* set flags b */
	b.th_flags = TH_SYN | TH_ACK;
	if (b.th_flags & TH_SYN) putchar('S');




	/*
	 * chat gpt output:
	 *
	 * Print TCP flags in tcpdump style.
	 *
	 * Flags are displayed as a sequence of characters:
	 *
	 *   S  - SYN (synchronize)
	 *   F  - FIN (finish)
	 *   R  - RST (reset)
	 *   P  - PSH (push)
	 *   U  - URG (urgent)
	 *   E  - ECE (ECN-echo)
	 *   W  - CWR (congestion window reduced)
	 *   .  - ACK (acknowledge)
	 *   e  - AE  (??)
	 *
	 *   https://github.com/the-tcpdump-group/tcpdump/blob/master/print-tcp.c
	 */
	if (b.th_flags & TH_SYN) putchar('S');
	if (b.th_flags & TH_FIN) putchar('F');
	if (b.th_flags & TH_RST) putchar('R');
	if (b.th_flags & TH_PUSH) putchar('P');
	if (b.th_flags & TH_ACK) putchar('.');
	if (b.th_flags & TH_URG) putchar('U');
	/*
	 * not found ece and cwr flags
	 * if (b.th_flags & TH_ECE) putchar('E');
	 * if (b.th_flags & TH_CWR) putchar('W');
	*/
	putchar(0x0a);

	return 0;
}
