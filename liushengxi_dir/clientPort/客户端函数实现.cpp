/*************************************************************************
	> File Name: 函数实现.cpp
	> Author: 
	> Mail: 
	> Created Time: 2018年02月03日 星期六 17时38分44秒
 ************************************************************************/

#include"ClientMyhead.h"
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
    client_msg.flag = 1 ;
    send(conn_fd,&client_msg,sizeof(TT),0);
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
        case  666 :
            //私聊消息
            cout << "massage.str == " << massage.str  << endl ;
            break;
        }
    }
}

