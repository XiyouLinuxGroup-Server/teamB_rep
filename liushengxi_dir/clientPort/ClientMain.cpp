/*************************************************************************
	> File Name: main.cpp
	> Author: 
	> Mail: 
	> Created Time: 2018年02月03日 星期六 17时42分16秒
 ************************************************************************/
#include"../myhead.h"
using namespace std ;
int main(int argc ,char **argv){
    if( argc != 3  )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    const int port = atoi( argv[2] );

	Myclient my(ip,port) ; //连接我的服务器

    pthread_t thid;
    pthread_create(&thid,NULL, my_recv,(void *)&my.conn_fd) ;  //my_recv 只需要套接字 

    TT arg = my.downloadFile(); //打印正在下载
    sleep(5) ;
    my.Mergefiles(arg);
	return 0 ;
}

