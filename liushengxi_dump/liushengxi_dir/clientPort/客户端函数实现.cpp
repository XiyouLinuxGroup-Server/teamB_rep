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
    CONNFD = conn_fd ;
}
Myclient::~Myclient(){
	close(conn_fd);
}
int Myclient::downloadFile(){ 
    TT client_msg ;
    memset(&client_msg,0,sizeof(TT));
    cout << "请 输  入 你 想 要 下 载 的 文 件 名   "  ;
    cin >> client_msg.filename ;
    tag:cout << "请 输  入 线 程 下 载 数 量  " ;
    cin >> client_msg.threadCount  ;
    if(client_msg.threadCount  < 1 )
        goto tag ;

    condTag.set();
    client_msg.flag = 0 ;
    send(conn_fd,&client_msg,sizeof(TT),0);

    if(condTag.timewait() == false  ){  //超时
        condTag.free_cond();
        return 0;
    }
    condTag.free_cond();

    int rret ;
    pthread_t tids[client_msg.threadCount];  //线程id  4   2
    TT *param ;
    for( int i = 0; i < client_msg.threadCount ; ++i )  
    {  
        param = new TT(client_msg) ;
        param->temp = i ;
        rret = pthread_create( &tids[i], nullptr, realdownloadFile, (void *)param); //开线程
        if( rret != 0 ) //创建线程成功返回0  
        {  
           cout << "pthread_create error:error_code=" << rret << endl;  
        }  
    }
    // 等待所有线程执行完 用不用这样呐 ？ 
    int ret[client_msg.threadCount] ;
    void *status[client_msg.threadCount];  
    for(unsigned j = 0; j != client_msg.threadCount; ++j )  
    {  
        ret[j] = pthread_join(tids[j], &status[j]);  
        if(ret[j] != 0)  
        {  
            if(ret[j] == ESRCH)  
            {  
                cout << "pthread_join():ESRCH 没有找到与给定线程ID相对应的线程" << endl;  
            }  
            else if(ret[j] == EDEADLK)  
            {  
                cout << "pthread_join():EDEADLKI 产生死锁" << endl;  
            }  
            else if(ret[j] == EINVAL)  
            {  
                cout << "pthread_join():EINVAL 与给定的县城ID相对应的线程是分离线程" << endl;  
            }  
            else  
            {  
                cout << "pthread_join():unknow error" << endl;  
            }  
            exit(-1);  
        }  
    }  
}
void *realdownloadFile(void *arg){   //线程下载文件
    printf("------------------------------------------------\n");
    TT client_msg = *(TT *)arg ; 
    client_msg.flag = 1  ;
    // printf("client_msg.filename == %s \n",client_msg.filename);
    // printf("client_msg.temp == %d \n",client_msg.temp);
    // // printf("client_msg.BityCount == %d \n",client_msg.BiteCount);
    // printf("client_msg.flag == %d \n",client_msg.flag);
    // printf("client_msg.threadCount == %d \n",client_msg.threadCount);
    // // printf("client_msg.str == %s \n",client_msg.str);
    printf("send return is %d \n",send(CONNFD,&client_msg,sizeof(TT),0));
    delete static_cast<TT *>(arg) ;
    pthread_exit(NULL);
}

void *my_recv(void* args)  //静态成员具有类的数据成员 conn_fd 
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
            condTag.signal() ;
            break;
        case 1:
        //接受文件 
            keep_file(massage);
            break ;
        case  1101  :
            //私聊消息
            cout << "客户端接受到数据 ： flag ==  "<< massage.flag   << endl ;
            break;
        }
    }
}
int keep_file(TT client_msg)
{
    char file_name[100];
    memset(file_name,0,sizeof(file_name));
    ofstream outfile ;

    // // printf("client_msg.filename == %s \n",client_msg.filename);
    // // printf("client_msg.temp == %d \n",client_msg.temp);
    // // printf("client_msg.BityCount == %d \n",client_msg.BiteCount);
    // // printf("client_msg.flag == %d \n",client_msg.flag);
    // // printf("client_msg.threadCount == %d \n",client_msg.threadCount);
    // printf("client_msg.str == %s \n",client_msg.str);
  
    sprintf(file_name,"刘生玺%d",client_msg.temp ); // 0  1 2 3 
    outfile.open(file_name, ios::out);  //只写 
    if(!outfile.is_open ())
        cout << "Open file failure" << endl;
    outfile.write(client_msg.str,client_msg.BiteCount);
    outfile.close();
}

