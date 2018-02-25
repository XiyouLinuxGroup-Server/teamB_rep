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
#include <sys/types.h>
#include <dirent.h>
#include<sys/time.h>


static const char* server_ip= "127.0.0.1" ;
#define SERVER_PORT  5201  

#define    MAX_EVENT_NUMBER   1024 
#define    LISTENQ      1024 
#define    MAXSIZE  52
#define  MAXSIZESTR  1024

struct TT{ //消息信息 
	int flag ;  // flag== 0 ,表示需要传文件 , 
	//flag == 1 正式开始传输
	int temp ; // 1 2 3 4 
	unsigned threadCount ; //线程数目 4
    unsigned size ; //每一段的大小字节数 
	unsigned BiteCount ; //每次发多大的包 
	char filename[MAXSIZE] ; //要请求的文件名 
	char str[MAXSIZESTR] ; //读取文件数据
};

struct fds
{
   int epollfd;
   int sockfd;
};
class Myserver{  
public:
	Myserver() ;  // 构造函数，初始化服务器
private:
	int setnonblocking( int fd );
	void addfd( int epollfd, int fd, bool oneshot ) ;
};
void  *worker(void  *arg) ; //线程函数
void reset_oneshot( int epollfd, int fd ) ; //worker 里面所使用的函数
int sure(TT server_msg,int conn_fd);
int send_file(TT server_msg ,const int &conn_fd); 



////////////////////////////////////////客户端头文件//////////////////////////////////////////////


class Myclient {   
	public:
	int conn_fd ;
    int threadCount ;
    struct sockaddr_in  server_address ;
	Myclient(const char *ip ,const int port );  // 构造函数
	~Myclient(); //析构函数
	int downloadFile();
};
void *my_recv(void* args) ;
void *realdownloadFile(void *arg) ;//线程函数
static int CONNFD ; //客户端套接字
static int section_size ;
int keep_file(TT client_msg) ;



class cond  //所需要用的条件变量
{
public:
    cond() = default ;
	int set()
    {
        if( pthread_mutex_init( &m_mutex, NULL ) != 0 )
        {
            throw std::exception();
        }
        if ( pthread_cond_init( &m_cond, NULL ) != 0 )
        {
            pthread_mutex_destroy( &m_mutex );
            throw std::exception();
        }
    }
    int free_cond()
    {
        pthread_mutex_destroy( &m_mutex );
        pthread_cond_destroy( &m_cond );
    }
    bool timewait()
    {
		
        int ret = 0;
		struct timeval now ;
		struct timespec outtime ;
        pthread_mutex_lock( &m_mutex );
        // ret = pthread_cond_wait( &m_cond, &m_mutex );
		gettimeofday(&now, NULL);
		outtime.tv_sec = now.tv_sec + 1 ;
		//outtime.tv_nsec = now.tv_usec * 1000;
		ret = pthread_cond_timedwait(&m_cond, &m_mutex, &outtime) ;
        pthread_mutex_unlock( &m_mutex );    

        if(ret == ETIMEDOUT )
            return false ;
        else  {      
            return true   ;
        }
    }
    bool signal()
    {
        return pthread_cond_signal( &m_cond ) == 0;
    }

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
static cond condTag ;
static void print(TT msg){
    printf("filename == %s \n",msg.filename);
    printf("temp == %d \n",msg.temp);
    printf("BityCount == %d \n",msg.BiteCount);
    printf("flag == %d \n",msg.flag);
    printf("threadCount == %d \n",msg.threadCount);
    printf("str == %s \n",msg.str);
    printf("size == %d \n",msg.size);
}

#endif
