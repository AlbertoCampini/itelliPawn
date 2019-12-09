#include <stdio.h>
#include <unistd.h>
#define SO_NUM_P 10

int main(){
    pid_t pidChild;
    int i;

    for(i = 0; i< SO_NUM_P; i++){
        if(fork() == 0){
            printf("ciao sonbo la pedina %d %d \n", i, getppid());
            break;
        }

    }
    while((pidChild = wait(NULL)) != -1) {

    }


}
