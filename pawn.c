#include "lib/core.h"
#include "lib/strategy.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

int main(int argc, char *argv[]) {
    int i, SO_BASE, SO_ALTEZZA, SO_MIN_HOLD_NSEC, gamerName, nMoves, posInMatrix, idSemMatrix, idMatrix, idMsgPawns, idSemSyncRound;
    int *matrix;
    SyncPawn syncGamer; /*Ricevo dal Gamer*/
    struct timespec tim;


    if(argc != (ARGS_TO_PASS_OF_PAWNS - 1)) {
        printf("Parametri passati insufficienti");
        return 0;
    }

    SO_BASE = readConfig("SO_BASE", HARD_MODE, CONF_FILE_PATH);
    if(SO_BASE < 0){ ERROR; return 0; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", HARD_MODE, CONF_FILE_PATH);
    if(SO_ALTEZZA < 0){ ERROR; return 0; }
    SO_MIN_HOLD_NSEC = readConfig("SO_MIN_HOLD_NSEC", HARD_MODE, CONF_FILE_PATH);
    if(SO_MIN_HOLD_NSEC < 0){ ERROR; return 0; }

    tim.tv_sec = 0;
    tim.tv_nsec = SO_MIN_HOLD_NSEC;

    sscanf(argv[1], "%d", &idMatrix);
    sscanf(argv[2], "%d", &idSemMatrix);
    sscanf(argv[3], "%d", &posInMatrix);
    sscanf(argv[4], "%d", &idMsgPawns);
    sscanf(argv[5], "%d", &idSemSyncRound);
    sscanf(argv[6], "%d", &gamerName);

    /*Attacco l'area del Matrix*/
    matrix = (int *)attachSHM(idMatrix);
    if(matrix == (void *)-1) {
        printf("Errore attach Matrix: ");ERROR;
        return 0;
    }

    /*Devo ricevere le configurazioni SyncPawn dal Gamer*/
    if(!receiveMessageToGamer(idMsgPawns, getpid(), &syncGamer)) { ERROR; return 0; }
    nMoves = syncGamer.nMoves;

    /*Decremento il SEM3 di -1 per dichiarare al Master che ho ricevuto la strategia*/
    if(!modifySem(idSemSyncRound, 2, -1)) { ERROR; return 0; }

    /*Attendo l'inizio round*/
    if(!waitSem(idSemSyncRound, 3)) {ERROR; return 0;}

    for(i = 0; i < nMoves; i++) {
        *(matrix + posInMatrix) = 0;
        posInMatrix = movesStrategy(syncGamer.strategy, idSemMatrix, posInMatrix, SO_BASE, SO_ALTEZZA);
        *(matrix + posInMatrix) = gamerName;
        nanosleep(&tim, NULL);

    }

    return 0;
}
