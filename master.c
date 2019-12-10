#include "core/core.h"
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define CONF_FILE_PATH "./config"
#define NAME "gamer"
#define ERROR printLastError() return 0

int main(){
    pid_t pidChild;
    int i, SO_NUM_G, SO_BASE, SO_ALTEZZA;
    char * args[4] = {NAME};
    int shm_id;

    //Leggo dal file di config
    SO_NUM_G = readConfig("SO_NUM_G", EASY_MODE, CONF_FILE_PATH);
    if(SO_NUM_G < 0){ ERROR; }
    SO_BASE = readConfig("SO_BASE", EASY_MODE, CONF_FILE_PATH);
    if(SO_BASE < 0){ ERROR; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", EASY_MODE, CONF_FILE_PATH);
    if(SO_ALTEZZA < 0){ ERROR; }

    //ottengo il puntatore alla mem condivisa di dimensione SO_ALTEZZA * SO_BASE
    shm_id = shmget(IPC_PRIVATE, SO_ALTEZZA*SO_BASE*sizeof(int), IPC_CREAT | 0666);


    for(i = 0; i < SO_NUM_G; i++){
        if(fork() == 0) {
            printf("Giocatore %d con pid %d \n", i, getppid());
            execve(NAME,args,NULL);
            break;
        }
    }

    while((pidChild = wait(NULL)) != -1) {
    }

    return 0;
}
