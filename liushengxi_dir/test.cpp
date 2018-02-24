#include"myhead.h"
using namespace std ;
int main(void){
    char str[512];
    for(int i = 2 ;i<=10 ;i++ ){
        sprintf(str,"cat %d.txt >> 1.txt ",i);
        system(str);
    }
}