#include "lib/core.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

int main(int argc, char *argv[]) {
    int i, SO_NUM_P, SO_NUM_G, SO_BASE, SO_ALTEZZA, posInMatrix, idSemMatrix, idMatrix, idMsgPawns, idSemSyncRound;
    SyncPawn syncPawn;

    if(argc != (ARGS_TO_PASS_OF_PAWNS - 1)) {
        printf("Parametri passati insufficienti");
        return 0;
    }

    sscanf(argv[1], "%d", &idMatrix);
    sscanf(argv[2], "%d", &idSemMatrix);
    sscanf(argv[3], "%d", &posInMatrix);
    sscanf(argv[4], "%d", &idMsgPawns);
    sscanf(argv[5], "%d", &idSemSyncRound);

    /*Devo ricevere le configurazioni SyncPawn dal Gamer*/
    if(!receiveMessageToGamer(idMsgPawns, getpid(), &syncPawn)) { ERROR; return 0; }

    /*Decremento il SEM3 di -1 per dichiarare al Master che ho ricevuto la strategia*/
    if(!modifySem(idSemSyncRound, 2, -1)) { ERROR; return 0; }

    /*Attendo l'inizio round*/

    return 0;
}
