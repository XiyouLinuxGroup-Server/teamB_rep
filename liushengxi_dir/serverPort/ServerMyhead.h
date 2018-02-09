/*************************************************************************
	> File Name: myhead.h
	> Author: 
	> Mail: 
	> Created Time: 2018年02月03日 星期六 16时01分59秒
 ************************************************************************/

#ifndef _MYHEAD_H
#define _MYHEAD_H
/*目标：
0.图形界面 
1.多线程上传和下载 
2.断点续传
3.上传和下载的目录 
4. 直接 url 下载 */

#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include <libgen.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<arpa/inet.h>
#include<errno.h>
#include<stdlib.h>
#include <sys/epoll.h>
#include<pthread.h>
#include<signal.h>
#include<assert.h>

#define SERVER_IP  "127.0.0.1" 
#define SERVER_PORT  5201  

#define    MAX_EVENTS_NUMBER   1024 
#define    LISTENQ      1024 

struct TT{ //消息信息 
	int flag ;  // flag== 1 ,表示需要传文件 , 
	//flag == 2 正式开始传输
	unsigned threadCount  = 1 ; //线程数目
	unsigned BiteCount ; //每次发多大的包 
	std::string filename ; //要请求的文件名 
	std::fstream start ;
	std::string str ; //读取文件数据
};

int send_file(TT server_msg ,int conn_fd) ; 
class Myclass{  //初始化服务器
    int listenfd ;
    struct sockaddr_in address ;
	struct epoll_event ev, events[MAX_EVENTS_NUMBER] ;
	static void  *fun(void *arg) ;  //处理传文件的逻辑
    friend int send_file(TT server_msg ,int conn_fd) ; 
public:
	Myclass();  // 构造函数
	~Myclass(); //析构函数
	int setnonblocking( int fd );
	void addfd( int epollfd, int fd, bool oneshot ) ;
};
#endif
