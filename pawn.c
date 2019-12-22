#include "lib/core.h"
#include "lib/strategy.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

static int i, points, nMoves, gamerName, SO_BASE, SO_ALTEZZA, idMsgPawns, idSemMatrix, idSemSyncRound, posInMatrix, alreadySend, alreadyMove1 = 0, alreadyMove2 = 0;
static int *matrix;
static SyncPawn syncGamer; /*Ricevo dal Gamer*/

static void timeoutHandle (int sig) {
    /*Ricostruisco la situazione dei movimenti*/
    /*if(!alreadyMove1) {
        printf("handle alreadyMove1\n");
        *(matrix + posInMatrix) = 0;
        posInMatrix = movesStrategy(syncGamer.strategy, idSemMatrix, idSemSyncRound, posInMatrix, SO_BASE, SO_ALTEZZA);
        if(posInMatrix >= 0) {
            if(*(matrix + posInMatrix) < 0) {
                points += (*(matrix + posInMatrix) * -1);
                if(!modifySem(idSemSyncRound, 4, -1)) { ERROR; }
            }
            *(matrix + posInMatrix) = gamerName;
        }
    }
    if(!alreadyMove2) {
        printf("handle alreadyMove2\n");
        if(posInMatrix >= 0) {
            if(*(matrix + posInMatrix) < 0) {
                points += (*(matrix + posInMatrix) * -1);
                if(!modifySem(idSemSyncRound, 4, -1)) { ERROR; }
            }
            *(matrix + posInMatrix) = gamerName;
        }
    }*/

    /*Se non ho ancora mandato il Msg al Gamer lo faccio da qui*/
    if(!alreadySend) {
        ResultRound resultRound;
        resultRound.points = points;
        resultRound.nMovesLeft = nMoves - i;
        resultRound.nMovesDo = i;
        resultRound.order = syncGamer.order;
        if(!sendMessageResultRound(idMsgPawns, 1, resultRound)) {
            ERROR;
        }
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    int SO_MIN_HOLD_NSEC, idMatrix;
    ResultRound resultRound;
    struct timespec tim;

    sigset_t maskSignal;
    struct sigaction signalAct;

    if(argc != (ARGS_TO_PASS_OF_PAWNS - 1)) {
        printf("Parametri passati insufficienti");
        return 0;
    }

    /*Abilito il segnale SIGUSR1*/
    sigaddset(&maskSignal, SIGUSR1);
    if(sigprocmask(SIG_UNBLOCK, &maskSignal, NULL) < 0) {
        ERROR;
        return 0;
    }
    /*imposto l'handler che verra eseguito quando chiamo SIGUSR1*/
    signalAct.sa_handler = timeoutHandle;
    signalAct.sa_flags = 0;
    signalAct.sa_mask = maskSignal;
    if(sigaction(SIGUSR1, &signalAct, 0) < 0) {
        ERROR;
        return 0;
    }

    //signal(SIGUSR1, timeoutHandle);

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

    do {
         /*Devo ricevere le configurazioni SyncPawn dal Gamer*/
        if(!receiveMessageToGamer(idMsgPawns, getpid(), &syncGamer)) { ERROR; return 0; }
        nMoves = syncGamer.nMoves;
        alreadySend = 0;

        /*Decremento il SEM3 di -1 per dichiarare al Master che ho ricevuto la strategia*/
        if(!modifySem(idSemSyncRound, 2, -1)) { ERROR; return 0; }
        printf("Decremento SEM3 (%d)\n", getValueOfSem(idSemSyncRound, 2));

        /*Attendo l'inizio round*/
        if(!waitSem(idSemSyncRound, 3)) {ERROR; return 0;}

        i = 0, points = 0;
        while(i < nMoves && !waitSemWithoutWait(idSemSyncRound, 4)) {
            /*Pulisco la posizione precedente*/
            alreadyMove1 = 0;
            //*(matrix + posInMatrix) = 0;

            /*Trovo la nuova posizione*/
            posInMatrix = movesStrategy(syncGamer.strategy, idSemMatrix, idSemSyncRound, posInMatrix, SO_BASE, SO_ALTEZZA);
            alreadyMove1 = 1;

            alreadyMove2 = 0;
            if(posInMatrix >= 0) {
                if(*(matrix + posInMatrix) < 0) {
                    /*Ho preso una Flags*/
                    points += (*(matrix + posInMatrix) * -1);
                    if(!modifySem(idSemSyncRound, 4, -1)) { ERROR; }
                    //printf("Ho preso la bandierina %d (%d, %d, %d)\n", posInMatrix, gamerName, nMoves - i, getValueOfSem(idSemSyncRound, 4));
                }

                *(matrix + posInMatrix) = gamerName;
                alreadyMove2 = 1;

                nanosleep(&tim, NULL);
                i++;
            }
        }

        /*Invio il resoconto al mio Gamer*/
        resultRound.order = syncGamer.order;
        resultRound.points = points;
        resultRound.nMovesLeft = nMoves - i;
        resultRound.nMovesDo = i;
        if(!sendMessageResultRound(idMsgPawns, 1, resultRound)) {
            ERROR;
        }
        alreadySend = 1;
    } while(1);

    return 0;
}
