
#include"../myhead.h"
using namespace std ;
int Myserver::setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}
void Myserver::addfd( int epollfd, int fd, bool oneshot =false )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if( oneshot )
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}
void reset_oneshot( int epollfd, int fd )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
}
Myserver::Myserver(){
    int ret = 0 ;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET ;
    inet_pton( AF_INET, server_ip, &address.sin_addr );
    address.sin_port = htons(SERVER_PORT);


    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 );

    int optval  =  1  ; //设置该套接字使之可以重新绑定端口
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(int *)&optval,sizeof(int))    < 0)
        printf("setsocketopt failed \n ");

    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 );

    ret = listen( listenfd, LISTENQ );
    assert( ret != -1 );

    epoll_event events[ MAX_EVENT_NUMBER ];
    int epollfd = epoll_create( 256 );
    assert( epollfd != -1 );
    addfd( epollfd, listenfd, false );//false 表示不对 epoll 启用 ET 模式 

    while( 1 )
    {
        int ret = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
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
                pthread_t thread;
                fds fds_for_new_worker;
                fds_for_new_worker.epollfd = epollfd;
                fds_for_new_worker.sockfd = sockfd;
                pthread_create( &thread, NULL, worker, ( void* )&fds_for_new_worker );
            }
            else
            {
                printf( "something else happened \n" );
            }
        }
    }
    close( listenfd );
}

void* worker( void* arg ) //线程函数
{
    TT server_msg ;
    memset(&server_msg,0,sizeof(TT)) ;

    const int sockfd = ( (fds*)arg )->sockfd;
    int epollfd = ( (fds*)arg )->epollfd;

    //printf( "start new thread to receive data on fd: %d\n", sockfd );
    while( 1 )
    {
        int ret = recv(sockfd,&server_msg,sizeof(TT),0) ; 
        if( ret == 0 )
        {
            close( sockfd );
            printf( "foreiner closed the connection\n" );
            break;
        }
        else if( ret < 0 )
        {
            if( errno == EAGAIN )
            {
                reset_oneshot( epollfd, sockfd );
                printf( "read later\n" );
                break;
            }
        }
        else
        {
            printf("***********************************flag  ==  %d\n",server_msg.flag);
            printf("temp  ==  %d\n",server_msg.temp );
            switch(server_msg.flag)
            {
                case 0: sure(server_msg,sockfd);   break ;      
                case 1: send_file(server_msg,sockfd);    break ;  
                //110 为测试代码
                case 110:   server_msg.flag = 1101 ; 
                            send(sockfd,&server_msg,sizeof(TT),0);
                            break ;
                default: break ;
            }

        }
    }
    // printf( "end thread receiving data on fd: %d\n", sockfd );
}

int  send_file(TT server_msg  ,const int &conn_fd ){ //flag==1 
    print(server_msg);
    char name[512];
    sprintf(name,"./file/%s",server_msg.filename);
    int file_fd = open(name,O_RDONLY) ;
    if(file_fd < 0 )
        cout << "create file failure  "<< endl ;

    if(lseek(file_fd,server_msg.size*server_msg.temp,SEEK_SET) < 0)
        cout << "lseek is failed  " << endl ;

    int sum = 0 ,file_len = 0 ;
    char read_buf[MAXSIZESTR]; //1024 
    while(sum != server_msg.size )  
    {
        printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
        memset(read_buf,0,sizeof(read_buf));
        memset(server_msg.str,0,sizeof(server_msg.str));

        file_len = read(file_fd,read_buf,4) ;

        memcpy(server_msg.str,read_buf,file_len);    //把文件内容拷贝到client.msg.str
        cout << "server_msg.str == "<< server_msg.str  << endl ;
        sum = sum + file_len ;
        server_msg.BiteCount = file_len ;
        server_msg.flag = 1 ;

        send(conn_fd,&server_msg,sizeof(TT),0) ;
    }
    close(file_fd);
}
int sure(TT server_msg,int conn_fd){
    //1.判断文件是否存在 ？
    char path[MAXSIZE] ="./file" ;
    DIR *dir ;
    struct dirent *ptr;
    if(   (dir=opendir(path))  == NULL  )
    {
        perror("opendir");
    }
    char name[512];
    while( ( ptr = readdir(dir) )  != NULL ){
        if(strcmp(ptr->d_name,server_msg.filename) == 0 ) {  
            //说明存在该文件,等待线程申请下载,发过来的还有线程数目，计算有多少个字节，客户端创建多少个文件
            sprintf(name,"./file/%s",server_msg.filename);
            int file_fd = open(name,O_RDONLY) ;
            if(file_fd < 0 )
                cout << "create file failure  "<< endl ;
            int filelen = lseek(file_fd,0L,SEEK_END);    
            cout << "filelen == " << filelen << endl ;
            server_msg.temp = filelen / server_msg.threadCount ;
            close(file_fd);
            strcpy(server_msg.str,"开 始 下 载 ------------\n"); 
            server_msg.flag = 666 ;
            send(conn_fd,&server_msg,sizeof(TT),0);
            closedir(dir);
            return 0;
        }
    }
    // 出循环代表不存在
    closedir(dir);
    strcpy(server_msg.str,"该文件在服务器上不存在，请核实后重新下载\n"); 
    server_msg.flag = 999 ;
    send(conn_fd,&server_msg,sizeof(TT),0);
}
