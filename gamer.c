#include "lib/core.h"
#include "lib/strategy.h"
#include <unistd.h>

#define CONF_FILE_PATH "./config"

int main(int argc, char *argv[]) {
    srand(time(NULL));

    pid_t pidChildKill;
    int i, posInMatrix, SO_NUM_P, SO_NUM_G, SO_BASE, SO_ALTEZZA, SO_N_MOVES, idMsgGamer, idSemMaster, idSemMatrix, idMatrix, idSemSyncRound, idMsgPawns, statusFork;
    int *matrix, *pidChild;
    char *argsToPawn[ARGS_TO_PASS_OF_PAWNS];
    char bufferIdSemMatrix[MAX_BUFF_SIZE], bufferIdMatrix[MAX_BUFF_SIZE], bufferPosInMatrix[MAX_BUFF_SIZE], bufferIdMsgPawns[MAX_BUFF_SIZE], bufferIdSemSyncRound[MAX_BUFF_SIZE], buffGamerName[MAX_BUFF_SIZE];
    SyncGamer syncMaster;   /*Ricevo dal Master*/
    SyncPawn syncPawn;      /*Invio al Pawn*/
    ResultRound resultRound, resultRoundGamer;

    sigset_t maskSignal;
    struct sigaction signalAct;

    if(argc != (ARGS_TO_PASS_OF_GAMER - 1)) {
        printf("Parametri passati insufficienti");
        return 0;
    }

    /*Imposto i segnali: blocco nella maschera SIGALRM*/
    /*if(sigprocmask(SIG_BLOCK, &maskSignal, NULL) < 0) {
        ERROR;
        return 0;
    }
    signalAct.sa_handler = timeoutHandle;
    signalAct.sa_flags = 0;
    signalAct.sa_mask = maskSignal;
    if(sigaction(SIGUSR1, &signalAct, 0) < 0) {
        ERROR;
        return 0;
    }*/

    /*Devo ricevere le configurazioni SyncGamer dal Master*/
    sscanf(argv[1], "%d", &idMsgGamer);
    if(!receiveMessageToMaster(idMsgGamer, getpid(), &syncMaster)) { ERROR; return 0; }

    /*Leggo dal file di config*/
    SO_NUM_G = readConfig("SO_NUM_G", HARD_MODE, CONF_FILE_PATH);
    if(SO_NUM_G < 0) { ERROR; return 0; }
    SO_NUM_P = readConfig("SO_NUM_P", HARD_MODE, CONF_FILE_PATH);
    if(SO_NUM_P < 0) { ERROR; return 0; }
    SO_BASE = readConfig("SO_BASE", HARD_MODE, CONF_FILE_PATH);
    if(SO_BASE < 0){ ERROR; return 0; }
    SO_ALTEZZA = readConfig("SO_ALTEZZA", HARD_MODE, CONF_FILE_PATH);
    if(SO_ALTEZZA < 0){ ERROR; return 0; }
    SO_N_MOVES = readConfig("SO_N_MOVES", HARD_MODE, CONF_FILE_PATH);
    if(SO_N_MOVES < 0){ ERROR; return 0; }

    /*Acquisisco in input gli argomenti passati da Master*/
    sscanf(argv[2], "%d", &idSemMaster);
    sscanf(argv[3], "%d", &idSemMatrix);
    sscanf(argv[4], "%d", &idMatrix);
    sscanf(argv[5], "%d", &idSemSyncRound);

    /*Istanzio la coda di messaggi privata tra Gamer e Pawns*/
    idMsgPawns = createMsgQueue(IPC_PRIVATE);
    if(!idMsgPawns) {
        printf("Errore creazione coda: ");ERROR;
        return 0;
    }

    /*Attacco l'area del Matrix*/
    matrix = (int *)attachSHM(idMatrix);
    if(matrix == (void *)-1) {
        printf("Errore attach Matrix: ");ERROR;
        return 0;
    }

    /*Allocazione dinamica di un array che contiene i PID dei Pawns*/
    pidChild = (int *)malloc(sizeof(int) * SO_NUM_P);

    for(i = 0; i < SO_NUM_P; i++) {
        /*1) Attesa sul mio semaforo*/
        if(!waitSem(idSemMaster, syncMaster.order)) {ERROR;}

        /*2) Generazione strategia e posizionamento pedina*/
        posInMatrix = positionStrategy(POS_STRATEGY_RANDOM, idSemMatrix, SO_BASE, SO_ALTEZZA);
        *(matrix + posInMatrix) = syncMaster.name;
        /*Fork dei Pawns passando con execve le coordinate su Matrix e semafori*/
        statusFork = fork();
        if(statusFork == 0) {
            sprintf(bufferIdSemMatrix, "%d", idSemMatrix);
            sprintf(bufferIdMatrix, "%d", idMatrix);
            sprintf(bufferPosInMatrix, "%d", posInMatrix);
            sprintf(bufferIdMsgPawns, "%d", idMsgPawns);
            sprintf(bufferIdSemSyncRound, "%d", idSemSyncRound);
            sprintf(buffGamerName, "%d", syncMaster.name);
            argsToPawn[0] = NAME_PAWN_PROCESS;
            argsToPawn[1] = bufferIdMatrix;
            argsToPawn[2] = bufferIdSemMatrix;
            argsToPawn[3] = bufferPosInMatrix;
            argsToPawn[4] = bufferIdMsgPawns;
            argsToPawn[5] = bufferIdSemSyncRound;
            argsToPawn[6] = buffGamerName;
            argsToPawn[7] = NULL;
            execve(NAME_PAWN_PROCESS, argsToPawn, NULL);
            break;
        } else if(statusFork == -1) {
            printf("Errore durante la fork\n");
            ERROR;
            exit(0);
        } else {
            /*Mi salvo il PID in un array*/
            *(pidChild + i) = statusFork;
            setpgid(statusFork, getppid());
        }

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

    /*SEM1: Il Gamer ha finito di posizionare tutte le sue pedine e decrementa di 1 il semaforo del Master*/
    if(!modifySem(idSemSyncRound, 0, -1)) {ERROR; return 0;}

    /*SEM2: Attendo che il Master posizioni le Flags*/
    if(!waitSem(idSemSyncRound, 1)) {ERROR;}

    /*Mando il mex a tutti i Pawns con la strategia da utilizzare (SyncPawns)*/
    for(i = 0; i < SO_NUM_P; i++) {
        syncPawn.strategy = MOVES_STRATEGY_RANDOM;
        syncPawn.nMoves = SO_N_MOVES;
        if(!sendMessageToPawns(idMsgPawns, *(pidChild + i), syncPawn)) {
            printf("Errore send messaggio: ");
            ERROR;
        }
    }
    free(pidChild);

    /*Attendo di ricevere SO_NUM_P messaggi di resoconto del round*/
    resultRoundGamer.points = 0;
    resultRoundGamer.nMovesLeft = SO_N_MOVES * SO_NUM_P;
    resultRoundGamer.nMovesDo = 0;
    for(i = 0; i < SO_NUM_P; i++) {
        if(!receiveMessageResultRound(idMsgPawns, 1, &resultRound)) { printf("ho spaccato la coda di messaggi\n");}
        else {
            resultRoundGamer.points += resultRound.points;
            resultRoundGamer.nMovesLeft -= resultRound.nMovesDo;
            resultRoundGamer.nMovesDo += resultRound.nMovesDo;
        }
    }
    /*Mando il resoconto del round al Master*/
    if(!sendMessageResultRound(idMsgGamer, 1, resultRoundGamer)) {
        ERROR;
    }

    /*Attendo la morte di tutte le mie Pawns*/
    while((pidChildKill = wait(NULL)) != -1) {
    }

    removeQueue(idMsgPawns);
    return 0;
}
