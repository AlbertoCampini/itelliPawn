#include "core/core.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

int main(){
    pid_t pidChild;
    int i, SO_NUM_P;

    //Leggo dal file di config
    SO_NUM_P = readConfig("SO_NUM_G", EASY_MODE, CONF_FILE_PATH);
    if(SO_NUM_P < 0) {
        printLastError();
        return 0;
    }

    for(i = 0; i < SO_NUM_P; i++){
        if(fork() == 0){
            printf("ciao sonbo la pedina %d %d \n", i, getppid());
            break;
        }
    }

    while((pidChild = wait(NULL)) != -1) {
    }

    return 0;
}
