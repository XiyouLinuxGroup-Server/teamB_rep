
#include"../myhead.h"
using namespace std ;
int Myserver::setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}
void Myserver::addfd( int epollfd, int fd, bool oneshot  )
{
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    if( oneshot )
    {
        ev.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &ev );
    setnonblocking( fd );
}
Myserver::Myserver(){
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET ;
    address.sin_port = htons(SERVER_PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = socket( AF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 );
    int optval  =  1  ; //设置该套接字使之可以重新绑定端口
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(int *)&optval,sizeof(int))    < 0)
        printf("setsocketopt failed \n ");

    int ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 );

    ret = listen( listenfd, LISTENQ );
    assert( ret != -1 );
    //epollfd epoll 句柄 
    int epollfd = epoll_create(  256 );
    assert( epollfd != -1 );

    addfd( epollfd, listenfd, false ); ///false 表示不对 epoll 启用 ET 模式 

    while( 1 )
    {
        ret = epoll_wait( epollfd, events, MAX_EVENTS_NUMBER, -1 );
        if ( ret < 0 )
        {
            printf( "epoll failure\n" );
            break;
        }
        for ( int i = 0; i < ret; i++ )
        {
            int sockfd = events[i].data.fd;
            if ( sockfd == listenfd )
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                printf("\t\t accept a connection from %s\n",inet_ntoa(client_address.sin_addr));
                addfd( epollfd, connfd, true );
            }
            else if ( events[i].events & EPOLLIN )
            {
                pthread_t thread ;
                if(pthread_create( &thread, NULL,fun ,(void *)&events[i].data.fd) < 0 )
                    printf("pthread_create is failed !! \n");
                pthread_detach(thread); //回收资源 
            }
            else
            {
                printf( "something else happened \n" );
            }
        }
    }
}
Myserver::~Myserver(){  //析构函数  
    close(listenfd) ;
}
void  *fun(void  *arg) 
{
    printf("liushengxi \n") ;
    TT server_msg ;
    const int conn_fd = *(int *)arg ; 
    memset(&server_msg,0,sizeof(TT)) ;
    int t = recv(conn_fd,&server_msg,sizeof(TT),0) ;   ///////////接受信息!!!!!!!!!!!!!!!
   /*  if(t == 0)  //处理异常
        warning_logout(server_msg ,conn_fd); */

    printf("****************************flag  ==  %d\n",server_msg.flag);
    switch(server_msg.flag)
    {
        case 0: sure(server_msg,conn_fd);   break ;      
        case 1:  cout << "999" << endl;  send_file(server_msg,conn_fd);    break ;  
        default: break ;
    }
    pthread_exit(NULL);  //线程退出  
}
int  send_file(TT server_msg  ,const int &conn_fd ){ //向客户端发文件 ，大小从 start 开始读取多少字节即可
    // fstream input("test.data");
    // input  >>  server_msg.str ; 
    printf("---------------------------------------------------\n");
    strcpy(server_msg.str,"XiyouLinux\n"); 
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    server_msg.flag = 666 ;
    send(conn_fd,&server_msg,sizeof(TT),0);
}
int sure(TT server_msg,int conn_fd){
    //1.判断文件是否存在？
    //2.调用send_file 函数进行传输
    char path[MAXSIZE] ="./file" ;
    DIR *dir ;
    struct dirent *ptr;
    if(   (dir=opendir(path))  == NULL  )
    {
        perror("opendir");
    }
    while( ( ptr = readdir(dir) )  != NULL ){
        if(strcmp(ptr->d_name,server_msg.filename) == 0 ) {  //说明存在该文件,等待线程申请下载 
                strcpy(server_msg.str,"开 始 下 载 ------------\n"); 
                server_msg.flag = 666 ;
                send(conn_fd,&server_msg,sizeof(TT),0);
                return 0;
        }
    }
    // 出循环代表不存在
    closedir(dir);
    strcpy(server_msg.str,"该文件在服务器上不存在，请核实后重新下载\n"); 
    server_msg.flag = 999 ;
    send(conn_fd,&server_msg,sizeof(TT),0);
}