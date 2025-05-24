#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

inline static size_t sunfill(struct sockaddr_un *s, const char *path)
{
	size_t pathlen;

	memset(s,0,sizeof(struct sockaddr_un));
	s->sun_family=AF_UNIX;
	
	if (path[0]=='\0')	/* abstract */
		pathlen=strlen(path+1)+1;
	else	
		pathlen=strlen(path)+1;

	memcpy(s->sun_path,path,pathlen);
	return (offsetof(struct sockaddr_un, sun_path)+pathlen);
}

const char *path="\0abc";
struct sockaddr_un sun;
size_t sunlen;
int fd;

#ifdef CLIENT
int main(void)
{
	sunlen=sunfill(&sun,path);
	fd=socket(AF_UNIX,SOCK_DGRAM,0);
	sendto(fd,"kek\n",5,0,(struct sockaddr*)&sun,sunlen);
	close(fd);
	return 0;
}
#else
int main(void)
{
	char buf[100];

	sunlen=sunfill(&sun,path);
	fd=socket(AF_UNIX,SOCK_DGRAM,0);

	//unlink(path);
	bind(fd,(struct sockaddr*)&sun,sunlen);
	for (;;) {
		if (read(fd,buf,sizeof buf)>0)
			printf("%s",buf);
	}

	close(fd);
	return 0;
}
#endif
