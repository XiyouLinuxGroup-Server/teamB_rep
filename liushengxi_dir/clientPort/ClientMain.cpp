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
    pthread_create(&thid,NULL, Myclient::my_recv,(void *)&my.conn_fd) ;  //my_recv 只需要套接字
    int choice ;
    do
    {
        //printf("\033c");
        // printf("\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t欢 迎 来 到 生 玺 下 载 系 统 \n\n\n");
        // printf("\t\t\t\t\t\t\t    0. 多 线 程 上 传 目 录 \n\n");
        // printf("\t\t\t\t\t\t\t    1. 多 线 程 上 传 文 件 \n\n");
        // printf("\t\t\t\t\t\t\t    2. 多 线 程 下 载 目 录 \n\n");
        printf("\t\t\t\t\t\t\t    3. 多 线 程 下 载 文 件\n\n");        
        // printf("\t\t\t\t\t\t\t    4. 多 线 程  URL 下 载 文 件 \n\n");        
        printf("\t\t\t\t\t\t\t    5.   取       消\n\n" );
        printf(      "\t\t\t\t\t\t\t    请 输 入 你 的 选 择 ： "         );
        cin >> choice ;
        switch(choice)
        {
            case 0: /*  my.uploadDir(); */  break ;
            case 1: /*  my.uploadFile();   */ break ;
            case 2:  /* my.downloadDir();    */    break ;
			case 3:   my.downloadFile();  sleep(3) ;break ;
			case 4:  /*  my.downloadFileFromURL();  */ break;
			case 5:     break;
            default :printf("\n\n\t\t\t\t\t\t\t输入有错～～～,请检查后重新输入!!!\n");sleep(1);break;
        }
    }while(choice !=  5 );     //取消
	return 0 ;
}

