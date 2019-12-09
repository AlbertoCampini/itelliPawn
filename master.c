#include <stdio.h>
#include <unistd.h>
#define SO_NUM_G 10
#define NAME "gamer"

int main(){
    pid_t pidChild;
    int i;
    char * args[4] = {NAME};

    for(i = 0; i< SO_NUM_G; i++){
        if(fork() == 0){
                printf("ciao sonbo il giocatore %d %d \n", i, getppid());
                execve(NAME,args,NULL);
            break;
        }


    }
    while((pidChild = wait(NULL)) != -1) {

    }


}
