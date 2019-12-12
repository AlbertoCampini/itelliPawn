#include "lib/core.h"
#include "lib/strategy.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

int main(int argc, char *argv[]) {
    srand(time(NULL));

    pid_t pidChild;
    int i, SO_NUM_P, SO_NUM_G, SO_BASE, SO_ALTEZZA, idMsgGamer, idSemMaster, idSemMatrix, idMatrix, statusFork;
    int *matrix;
    char *args[] = {NAME_PAWN_PROCESS, NULL};
    SyncGamer syncMaster;

    /* 1) Devo ricevere le configurazioni SyncGamer dal Master*/
    sscanf(argv[1], "%d", &idMsgGamer);
    if(!receiveMessage(idMsgGamer, getpid(), &syncMaster)) {
        ERROR;
        return 0;
    }

    /*Leggo dal file di config*/
    SO_NUM_G = readConfig("SO_NUM_G", EASY_MODE, CONF_FILE_PATH);
    if(SO_NUM_G < 0) { ERROR; return 0; }
    SO_NUM_P = readConfig("SO_NUM_P", EASY_MODE, CONF_FILE_PATH);
    if(SO_NUM_P < 0) { ERROR; return 0; }
    SO_BASE = readConfig("SO_BASE", EASY_MODE, CONF_FILE_PATH);
    if(SO_BASE < 0){ ERROR; return 0; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", EASY_MODE, CONF_FILE_PATH);
    if(SO_ALTEZZA < 0){ ERROR; return 0; }

    /*Acquisisco in input gli argomenti passati da Master*/
    sscanf(argv[2], "%d", &idSemMaster);
    sscanf(argv[3], "%d", &idSemMatrix);
    sscanf(argv[4], "%d", &idMatrix);

    matrix = (int *)attachSHM(idMatrix);
    if(matrix == (void *)-1) {
        printf("Errore attach Matrix: ");ERROR;
        return 0;
    }

    for(i = 0; i < SO_NUM_P; i++) {
        /*1) Attesa sul mio semaforo*/
        if(!waitSem(idSemMaster, syncMaster.order)) {ERROR;}

        /*2) Generazione strategia e posizionamento pedina*/
        //printf("Metto la pedina (%d)\n", syncMaster.order);
        *(matrix + positionStrategy(POS_STRATEGY_RANDOM, idSemMatrix, SO_BASE, SO_ALTEZZA)) = 1;


        /*3) Controllo sul giocatore successivo*/
        if((syncMaster.order + 1) < SO_NUM_G) {
            /*Sblocco il semaforo del giocatore successivo*/
            if(!modifySem(idSemMaster, (syncMaster.order + 1), -1)) {ERROR;}
        } else {
            /*Sblocco il semaforo del PRIMO giocatore successivo*/
            if(!modifySem(idSemMaster, 0, -1)) {ERROR;}
        }

        /*4) Blocco il mio semaforo*/
        if(!modifySem(idSemMaster, syncMaster.order, 1)) {ERROR;}
    }

    /*for(i = 0; i < SO_NUM_P; i++) {
        statusFork = fork();
        if(statusFork == 0) {
            //printf("Pedina %d con pid %d \n", i, getppid());
            execve(NAME_PAWN_PROCESS, args, NULL);
            break;
        } else if(statusFork == -1) {
            printf("Errore durante la fork\n");
            ERROR;
            break;
        }
    }

    while((pidChild = wait(NULL)) != -1) {
    }*/

    return 0;
}
