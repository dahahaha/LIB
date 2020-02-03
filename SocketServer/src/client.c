#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  BUFFER_SIZE 128

int main(int argc, char const *argv[])
{
	int sockfd;
	char buf[BUFFER_SIZE] = "HELLO LIBEV....";
	struct sockaddr_in servaddr;

	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == 1)
	{
		printf("socket creat error.......\n");
		return 0;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);


	if (connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) == -1)
	{
		printf("connect error\n");
		return 0;
	}

	while(1)
	{
		sleep(atoi(argv[3]));
		send(sockfd,buf,sizeof(buf),0);
	}

	return 0;
}