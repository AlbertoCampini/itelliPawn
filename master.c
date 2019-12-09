#include "core/core.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"
#define NAME "gamer"

int main(){
    pid_t pidChild;
    int i, SO_NUM_G;
    char * args[4] = {NAME};

    //Leggo dal file di config
    SO_NUM_G = readConfig("SO_NUM_G", EASY_MODE, CONF_FILE_PATH);
    if(SO_NUM_G < 0) {
        printLastError();
        return 0;
    }

    for(i = 0; i < SO_NUM_G; i++){
        if(fork() == 0) {
            printf("ciao sonbo il giocatore %d %d \n", i, getppid());
            execve(NAME,args,NULL);
            break;
        }
    }

    while((pidChild = wait(NULL)) != -1) {
    }

    return 0;
}
