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
TT Myclient::downloadFile(){ 
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
    send(conn_fd,&client_msg,sizeof(TT),0); //进行确认

    if(condTag.timewait() == false  ){  //超时
        condTag.free_cond();
        exit(-1);
    }
    condTag.free_cond();

/* 打印正在下载 */
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
           myerror(" pthread_create  failed ",__LINE__);
        }  
    }
    // 等待所有线程执行完
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
    return client_msg ; 
}
void *realdownloadFile(void *arg){   //线程下载文件
    TT client_msg = *(TT *)arg ; 
    client_msg.flag = 1  ; 
    client_msg.size = section_size ;

    send(CONNFD,&client_msg,sizeof(TT),0);

    delete static_cast<TT *>(arg) ;
    pthread_exit(NULL);
}

int Myclient::Mergefiles(TT client_msg){  //合并文件
    char str[512];
    for(int j = 1 ;j<= client_msg.threadCount ;j++){
        close(filefds[j]);     //关闭文件描述符
    }
    system("touch download");
    for(int i = 1 ;i<= client_msg.threadCount ;i++ ){
        memset(str,0,sizeof(str));
        sprintf(str,"cat %d.txt >>  download ",i);
        system(str);
    }
    system("rm *.txt") ;
    printf("<< -------------下 载 成 功 ，文件名为 download \n\n\n");
    return 0;
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
        case  5 :
            //一个字节一个字节的传输也太慢了呀
            cout << " Sorry !" << massage.str  << endl ;
            exit(-1) ;
        case  999 :
            //文件名检测失败
            cout << " Sorry !"<< massage.str  << endl ;
            break;
        case  666 :
            //文件名检测通过，创建对应的几个文件
            int file_fd ;
            char name[512];
            printf("massagr.temp == %d \n",massage.temp); //每段的大小
            section_size = massage.temp ;
            for(int i = 0 ;i < massage.threadCount ;i++ ){  //4 
                memset(name,0,sizeof(name));
                sprintf(name,"./%d.txt",i+1 ); 
                file_fd = open(name,O_TRUNC | O_CREAT |O_APPEND | O_WRONLY,S_IRUSR | S_IWUSR); 
                //所有者可写入,可读取 O_TRUNC:如果文件存在，且以可写的方式打开时，将文件清零
                if(file_fd < 0 )
                {
                    myerror("create file failed  ",__LINE__) ;
                }
                filefds[i+1] = file_fd ; 
            }
            cout << massage.str  << endl ;
            condTag.signal() ;
            break ;
        case 1:
        //接受文件 ,保存所对应的文件数据
            keep_file(massage); //temp 为 主线 
            break ;
        case  1101  :
            //测试代码 
            cout << "客户端接受到数据 ： flag ==  "<< massage.flag   << endl ;
            break;
        }
    }
}
int keep_file(TT client_msg)   //以temp归类
{
    // print(client_msg);
    
    if( write(filefds[client_msg.temp+1],client_msg.str,client_msg.BiteCount) < 0) 
        myerror("write file failed ",__LINE__ );
    return 0;
}

