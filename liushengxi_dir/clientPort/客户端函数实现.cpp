/*************************************************************************
	> File Name: 函数实现.cpp
	> Author: 
	> Mail: 
	> Created Time: 2018年02月03日 星期六 17时38分44秒
 ************************************************************************/

#include"../myhead.h"
using namespace std ;
Myclient::Myclient(const char *ip ,const int port ){
	bzero( &server_address, sizeof( server_address ) );
    server_address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &server_address.sin_addr );
    server_address.sin_port = htons( port );

    conn_fd = socket( AF_INET, SOCK_STREAM, 0 );
    assert( conn_fd >= 0 );
    if ( connect( conn_fd, ( struct sockaddr* )&server_address, sizeof( server_address ) ) < 0 )
    {
        printf( "connection failed\n" );
        close( conn_fd );
    }
}
Myclient::~Myclient(){
	close(conn_fd);
}
int Myclient::downloadFile(){ 
    TT client_msg ;
    cout << "请 输  入 你 想 要 下 载 的 文 件 名   "  ;
    cin >> client_msg.filename ;
    cout << "请 输  入 线 程 下 载 数 量  "  ;
    cin >> client_msg.threadCount  ;
    client_msg.flag = 0 ;
    send(conn_fd,&client_msg,sizeof(TT),0);
    while(1){
        sleep(3);
        if( ss == 1 )
            break ;
    }
    pthread_t tids[client_msg.threadCount];  //线程id  
    for( int i = 0; i < client_msg.threadCount ; ++i )  
    {  
        client_msg.threadCount = i ;//只需要标识在哪一段即可
        client_msg.connfd = conn_fd ;
        int ret = pthread_create( &tids[i], NULL, realdownloadFile, (void *)&client_msg); //开线程
        if( ret != 0 ) //创建线程成功返回0  
        {  
           cout << "pthread_create error:error_code=" << ret << endl;  
        }  
    }  
    pthread_exit( NULL ); //等待各个线程退出后，进程才结束，否则进程强制结束，线程处于未终止的状态  
}



void *realdownloadFile(void *arg){  //线程下载文件
    TT client_msg = *(TT *)arg ; 
    client_msg.flag = 1 ;
    send(client_msg.connfd,&client_msg,sizeof(TT),0) ;
}



void *Myclient::my_recv(void* args)  //静态成员具有类的数据成员 conn_fd 
{
    //只用来接收信息的子线程
    int I_conn_fd = *(int *)args ;
    while(1) {
        TT  massage ;
        memset(&massage ,0,sizeof(TT ));
        int re ;
        re = recv(I_conn_fd,&massage ,sizeof(TT ),0);
        if(re == 0) {
            puts("服务器开始维护,你已断开连接!");
            exit(0);
        } else if(re< 0){
            cout << "error in my_recv function "<< endl ;
            exit(0);
        }
        switch(massage.flag) {
        case  999 :
            //私聊消息
            cout << "sorry !"<< massage.str  << endl ;
            break;
        case  666 :
            //私聊消息
            cout << "good job ! "<< massage.str  << endl ;
            ss = 1 ;
            break;
        // case :
        //     break ;
        }
    }
}

