
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
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET ;
    if( oneshot )
    {
        ev.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &ev );
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
                fds fds_for_new_worker;
                fds_for_new_worker.epollfd = epollfd;
                fds_for_new_worker.sockfd = sockfd ;
                if(pthread_create( &thread, NULL,fun ,(void *)&fds_for_new_worker ) < 0 )
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

file::file(TT  &server_msg ,const int &fd){
    file_fd = fd ; 
    char file_name[MAXSIZE]; 
    strcpy(file_name,server_msg.filename); //filename = test.data  
    sprintf(file_name,"./file/%s",server_msg.filename);
    infile.open(file_name, ios::in);  //只读 
    if(!infile.is_open ())
        cout << "Open file failure" << endl;
    infile.seekg(0, ios::end);
    sum_len = infile.tellg(); 
    count = server_msg.temp ; //第几个线程 0 1 2 3 
    section_number = sum_len /server_msg.threadCount ; //注意会有余数的问题
    infile.seekg(0, ios::beg);
    infile.seekg(section_number*count,ios::beg);
}
file::~file(){
    infile.close();
}
int file::real_send_file(TT &server_msg) {  //正式发送文件
    int  local = 0 ,temp_len = 0  ;
    char read_buf[MAXSIZE];
    while(local != section_number ) //服务器不需要知道文件有多大，发完就完了啊 
    {
        memset(read_buf,0,sizeof(read_buf));
        infile.read(read_buf,128);
        temp_len = infile.gcount() ;
        memcpy(server_msg.str,read_buf,temp_len);    //把文件内容拷贝到server_msg.str
        local  = local + temp_len ;
        server_msg.BiteCount = temp_len ;
        send(file_fd,&server_msg,sizeof(TT),0) ;
        // //printf("**********************\n");
        // usleep(15000);
        // //usleep(10000); //error
    }
}

void  *fun(void  *arg) 
{
    TT server_msg ;
    const int conn_fd = ( (fds*)arg )->sockfd;
    int epollfd = ( (fds*)arg )->epollfd;
    memset(&server_msg,0,sizeof(TT)) ;
    while( 1 )
    {
        int ret = recv(conn_fd,&server_msg,sizeof(TT),0) ;
        if( ret == 0 )
        {
            close( conn_fd );
            printf( "foreiner closed the connection\n" );
            break;
        }
        else if( ret < 0 )
        {
            if( errno == EAGAIN )
            {
                reset_oneshot( epollfd ,conn_fd );
                break;
            }
        }
        else
        {
            printf("****************************flag  ==  %d\n",server_msg.flag);
            switch(server_msg.flag)
            {
                case 0: sure(server_msg,conn_fd);   break ;      
                case 1: send_file(server_msg,conn_fd);    break ;  
                case 110: cout << "666" << endl ;break ;
                default: break ;
            }
        }
    }
}
int  send_file(TT server_msg  ,const int &conn_fd ){ //向客户端发文件 ，大小从 start 开始读取多少字节即可
    printf("server_msg.filename == %s \n",server_msg.filename);
    printf("server_msg.temp == %d \n",server_msg.temp);
    printf("server_msg.BityCount == %d \n",server_msg.BiteCount);
    printf("server_msg.flag == %d \n",server_msg.flag);
    printf("server_msg.threadCount == %d \n",server_msg.threadCount);
    printf("server_msg.str == %s \n",server_msg.str);
    file myfile(server_msg,conn_fd);
    myfile.real_send_file(server_msg);


    server_msg.flag = 2 ; //表示一个线程传输完成
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
