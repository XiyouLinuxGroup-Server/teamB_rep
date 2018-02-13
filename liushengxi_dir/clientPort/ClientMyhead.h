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
#include<fstream>
#define    MAXSIZE  512 
struct TT{ //消息信息 
	int conn_fd ;
	int flag ;  // flag== 1 ,表示需要传文件 , 
	//flag == 2 正式开始传输
	unsigned threadCount ; //线程数目
	unsigned BiteCount ; //每次发多大的包 
	char filename[MAXSIZE] ; //要请求的文件名 
	char str[MAXSIZE] ; //读取文件数据
};

class Myclient {   
	public:
	int conn_fd ;
    struct sockaddr_in  server_address ;
	Myclient(const char *ip ,const int port );  // 构造函数
	~Myclient(); //析构函数
	int downloadFile();
	static void *my_recv(void* args) ;
};


#endif
