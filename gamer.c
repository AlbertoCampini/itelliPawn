#include "core/core.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

int main(int argc, char *argv[]) {
    pid_t pidChild;
    int i, SO_NUM_P, SO_NUM_G, idMsgGamer, idSemMaster, statusFork;
    char *args[] = {NAME_PAWN_PROCESS, NULL};
    SyncGamer syncMaster;

    /* 1) Devo ricevere le configurazioni SyncGamer dal Master*/
    sscanf(argv[1], "%d", &idMsgGamer);
    if(!receiveMessage(idMsgGamer, getpid(), &syncMaster)) {
        printLastError();
        return 0;
    }

    /*Leggo dal file di config*/
    SO_NUM_G = readConfig("SO_NUM_G", HARD_MODE, CONF_FILE_PATH);
    if(SO_NUM_G < 0) { printLastError(); return 0; }
    SO_NUM_P = readConfig("SO_NUM_P", EASY_MODE, CONF_FILE_PATH);
    if(SO_NUM_P < 0) { printLastError(); return 0; }

    sscanf(argv[2], "%d", &idSemMaster);
    for(i = 0; i < SO_NUM_P; i++) {
        /*Attesa sul mio semaforo*/
        if(!waitSem(idSemMaster, syncMaster.order)) {

        }
        /*Posizionamento pedina*/
        printf("Metto la pedina (%d)\n", syncMaster.order);
        /*Controllo sul giocatore successivo*/
        if((syncMaster.order + 1) < SO_NUM_G) {
            //Sblocco il semaforo del giocatore successivo*/
            if(!modifySem(idSemMaster, (syncMaster.order + 1), -1)) {

            }
        } else {
            /*Sblocco il semaforo del PRIMO giocatore successivo*/
            if(!modifySem(idSemMaster, 0, -1)) {

            }
        }
        /*Blocco il mio semaforo*/
        if(!modifySem(idSemMaster, syncMaster.order, 1)) {

        }
    }

    /*for(i = 0; i < SO_NUM_P; i++) {
        statusFork = fork();
        if(statusFork == 0) {
            //printf("Pedina %d con pid %d \n", i, getppid());
            execve(NAME_PAWN_PROCESS, args, NULL);
            break;
        } else if(statusFork == -1) {
            printf("Errore durante la fork\n");
            printLastError();
            break;
        }
    }

    while((pidChild = wait(NULL)) != -1) {
    }*/

    return 0;
}
