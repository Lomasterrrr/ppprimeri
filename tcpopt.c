#include <stdio.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

/*
 * VIberi svoi style TYPES:
 *
 * u_char,  uint8_t,  u8,  unsigned char,  __uint8,  byte,  BYTE,  __u8
 * u_short, uint16_t, u16, unsigned short, __uint16, word,  WORD,  __u16
 * u_int,   uint32_t, u32, unsigned int,   __uint32, dword, DWORD, __u32
 * u_long,  uint64_t, u64, [??],           __uint64, qword, QWORD, __u64
 */

struct tcp_opt_mss
{
	u_char etc[2]; // == u_char kind, len;
	u_short mss;
};

int main(void)
{
	struct tcp_opt_mss mss;
	struct tcphdr tcp;
	u_char reserved;
	u_int optlen;

	mss.etc[0]=2, mss.etc[1]=4, mss.mss=TCP_MSS;

	optlen=sizeof(struct tcp_opt_mss);
	reserved=0; /* no */

	tcp.th_ack=0;
	tcp.th_win=htons(TCP_MAXWIN);
	tcp.th_sport=htons(1234);
	tcp.th_flags=TH_SYN;
	tcp.th_seq=htonl(234);
	tcp.th_urp=0;
	tcp.th_dport=htons(80);
	tcp.th_off=(5+(optlen/4));
	tcp.th_x2=((reserved)?(reserved&0xFF):0);
	tcp.th_sum=0;

	return 0;
}

/*
 * https://github.com/nmap/nmap/blob/068dd4b0dfcf459c0948f05c6583cb17a49111a1/tcpip.cc#L639
 */
