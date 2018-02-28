#include"myhead.h"
using namespace std ;
int main(void){
    char str[512];
    for(int i = 2 ;i<= 48 ;i++ ){
        memset(str,0,sizeof(str));
        sprintf(str,"cat %d.txt >> 1.txt ",i);
        system(str);
    }
}
