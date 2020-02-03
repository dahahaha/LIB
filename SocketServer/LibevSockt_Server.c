#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ev.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>



#define MAX_FD 65535
#define CONNECT_IDLE_TIMEOUT 5 //s


struct client_context{

	/*TODO:do nothing*/
};

struct libev_context{

	struct ev_io watcher_io; //存储每个客户端的libev结构体
	struct ev_timer timer; //每个客户端timer
	struct client_context context; //存储每个客户端上下文信息
};


void accept_cb(EV_P_ ev_io *w,int revents);
void client_cb(EV_P_ ev_io *w,int revents);
void client_timer_cb(EV_P_ ev_timer *w,int revents);
//客户端连接数组
struct libev_context *client[MAX_FD];
int main(int argc, char const *argv[])
{
	

	int listenfd;
	int t1;

	//ev 事件初始化
	struct ev_loop *loop = EV_DEFAULT;
	// socket 监听地址
	struct sockaddr_in addr;
	//accept io 监听监视器
	struct ev_io accept_watcher;

	if (argc != 3)
	{
		printf("Usage:%s <IP> <PORT>\n",argv[0] );
		exit(-1);
	}

	for (int t1 = 0; t1 < MAX_FD; ++t1)
	{
		client[t1] = NULL;
	}

	if ((listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("Socket error\n");
		return 0;
	}

	//addr parm 参数设置
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	addr.sin_addr.s_addr = inet_addr(argv[1]);

	//bind listenfd&addr 
	if (bind(listenfd,(struct sockaddr *)&addr,sizeof(addr)) == -1)
	{
		printf("Bind error \n");
		return 0;
	}

	//listen 
	if (listen(listenfd,5) == -1)
	{
		printf("Listen error\n");
		return 0;
	}

	ev_io_init(&accept_watcher,accept_cb,listenfd,EV_READ);
	ev_io_start(loop,&accept_watcher);
	ev_run(loop,0);

	return 0;
}

// accept_cb 
void accept_cb(EV_P_ ev_io *w,int revents)
{
	int connect_fd = -1;
	struct sockaddr_in connect_addr;
	socklen_t client_len;

	if (EV_ERROR & revents)
	{
		printf("Error event in accept,code %d:%s\n",errno,strerror(errno));
		return ;
	}

	 // accept fd 
	connect_fd = accept(w->fd,(struct sockaddr*)(&connect_addr),&client_len);
	if (connect_fd < 0)
	{
		printf("Accept error ,code %d:%s\n",errno,strerror(errno) );
	}

	if(connect_fd > MAX_FD)
	{
		printf(" Max fd thread.connect_fd = %d\n",connect_fd);
		goto err_accept;
	}

	//clinet accept successed  
	client[connect_fd] = (struct libev_context *)malloc(sizeof(struct libev_context));
	if (client[connect_fd] == NULL)
	{
		printf("Malloc error code %d:%s\n",errno,strerror(errno) );
		goto err_accept;

	}
	printf("Clinet context alloc OK...\n");

	//add client task
	ev_io_init(&(client[connect_fd]->watcher_io) ,client_cb, connect_fd,EV_READ);
	ev_io_start(loop,&(client[connect_fd]->watcher_io));
	//client idle timer
	ev_timer_init(&(client[connect_fd]->timer),client_timer_cb,CONNECT_IDLE_TIMEOUT,0);
	ev_timer_start(loop,&(client[connect_fd]->timer));
	printf("Client connected success....\n");
	return ;

err_accept:

	close(w->fd);
	printf("listenfd closed...\n");
	return ;

}

//每个客户端task
void client_cb(struct ev_loop * loop,ev_io *w, int revents)
{
	char buf[1024];
	ssize_t readlen;

	struct sockaddr_in addrfrom;
	socklen_t peerlen = sizeof(addrfrom);


	client[w->fd]->timer.repeat = CONNECT_IDLE_TIMEOUT;
	ev_timer_again(loop,&(client[w->fd]->timer));

	bzero(buf,1024);

	if (EV_ERROR & revents)
	{
		printf("Read Errorc ,code%d:%s\n",errno,strerror(errno) );
		goto err_read;
	}

	//readlen = recv(w->fd,buf,1024,0);
	readlen = recvfrom(w->fd,buf,1024,0,(struct sockaddr *)&addrfrom,&peerlen);

	if (readlen < 0)
	{
		printf("Read Error ,code%d:%s\n",errno,strerror(errno) );
		goto err_read;
	}
	else if (readlen == 0)
	{
		int t = w->fd;
		printf("Disconnected, code %d: %s\n",errno, strerror(errno) );

		ev_io_stop(loop, &(client[t]->watcher_io));
		ev_timer_stop(loop,&(client[t]->timer));
		free(client[t]);
		client[t] = NULL;
		close(w->fd);
		printf("Context free OK .....\n");
	}
	else
	{
		printf("[%s:%d]%s\n",inet_ntoa(addrfrom.sin_addr),ntohs(addrfrom.sin_port),buf);
	}

	return ;

err_read:
	close(w->fd);
	printf("listenfd closed...\n");
	return ;
}

void client_timer_cb(struct ev_loop *loop,ev_timer *w,int revents)
{
	int  closefd;

	printf("CLient not alive ,closed...\n");

	//0地址对齐：从成员地址 结构体地址
	struct  libev_context *client_context;
	client_context = (struct libev_context *)( (char *)w -(int)(&(((struct libev_context *)0)->timer)) );

	closefd =client_context->watcher_io.fd;
	ev_timer_stop(loop,&(client_context->timer));
	ev_io_stop(loop,&(client_context->watcher_io));
	
	free(client_context);
	printf("Context free OK...\n");
	client_context = NULL;
	close(closefd);

}