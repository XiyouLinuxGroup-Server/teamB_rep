#include"myhead.h"
using namespace std ;
int main(void){
    char name[512] = "./file/test.txt";
    int file_fd = open(name,O_RDONLY) ;
    if(file_fd < 0 )
        cout << "create file failure  "<< endl ;
    int filelen = lseek(file_fd,0L,SEEK_END);    
    cout << "filelen == " << filelen << endl ;
    close(file_fd);
}