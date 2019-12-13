#include "core.h"

int generateRandom(int to, int from) {
    return rand() % from + to;
}
//Legge dal file di configurazioni il valore della char *config con la modalità
const int readConfig(char *config, int mode, const char *fPath) {
    FILE *fConf = fopen(fPath, "r");
    if (fConf == NULL) {
        return -1;
    }
    char line[MAX_LINE_CONF];
    while(fgets(line, sizeof(line), fConf) != NULL) {
        if(strcmp(line, "") != 0 && strcmp(line, "\n") != 0) {
            int modeRead = atoi(strtok(line, "."));
            char *configRead = strtok(NULL, ":");
            int valueRead = atoi(strtok(NULL, ":"));
            //void *value = &valueRead;

            if(strcmp(config, configRead) == 0) {
                if(mode == modeRead) {
                    return valueRead;
                }
            }
        }

    }
    fclose(fConf);
    return -1;
}
void printLastError() {
    printf("%s\n", strerror(errno));
}
void printMatrix(int *matrix, const int base, const int higth) {
    int i;
    printf("|");
    for(i = 0; i < (base * higth); i++) {
        if((i % base) == 0 && i != 0) {
            printf("\n");
            printf("|");
        }
        if(matrix[i] == 0) {
                printf(" |");
        } else if(matrix[i] == -1) {
            printf("\033[0;31m");
            printf("F");
            printf("\033[0m");
            printf("|");
        } else {
            printf("\033[0;35m");
            printf("%d", matrix[i]);
            printf("\033[0m");
            printf("|");
        }
    }
    printf("\n");
}


int createMsgQueue(key_t key) {
    int id = msgget(key, IPC_CREAT | IPC_EXCL);
    if(id < 0) {
        return 0;
    }
    return id;
}
int removeQueue(int id) {
    if(msgctl(id, IPC_RMID, NULL) < 0) {
        return 0;
    }
    return 1;
}
int sendMessage(int id, long msgType, SyncGamer syncGamer) {
    syncGamer.mtype = msgType;
    if(msgsnd(id, &syncGamer, (sizeof(SyncGamer) - sizeof(long)), 0) < 0) {
        return 0;
    }
    return 1;
}
int receiveMessage(int idMsg, long msgType, void *msg) {
    if(msgrcv(idMsg, msg, (sizeof(SyncGamer) - sizeof(long)), msgType, 0) < 0) {
        return 0;
    }
    return 1;
}


int createAndInitSems(key_t semKey, const int nSems, unsigned short valInit) {
    int idSem, i;
    if((idSem = semget(semKey, nSems, IPC_CREAT | IPC_EXCL)) < 0) {
        return 0;
    }

    union semun arg;
    unsigned short valsInit[nSems];
    for(i = 0; i < nSems; i++) {
        valsInit[i] = valInit;
    }
    arg.array = valsInit;

    if(semctl(idSem, 0, SETALL, arg) < 0) {
        return 0;
    }
    return idSem;
}
int removeSem(int semId) {
    if(semctl(semId, IPC_RMID, 0) < 0) {
        return 0;
    }
    return 1;
}
int modifySem(int semId, int semNum, const int num) {
    struct sembuf sOps[1];
    sOps[0].sem_num = semNum;
    sOps[0].sem_op = num;
    sOps[0].sem_flg = 0;
    if(semop(semId, sOps, 1) < 0) {
        return 0;
    }
    return 1;
}
int waitSem(int semId, int semNum) {
    struct sembuf sOps[1];
    sOps[0].sem_num = semNum;
    sOps[0].sem_op = 0;
    sOps[0].sem_flg = 0;
    if(semop(semId, sOps, 1) < 0) {
        return 0;
    }
    return 1;
}
int waitSemWithoutWait(int semId, int semNum) {
    struct sembuf sOps[1];
    sOps[0].sem_num = semNum;
    sOps[0].sem_op = 0;
    sOps[0].sem_flg = IPC_NOWAIT;
    if(semop(semId, sOps, 1) < 0) {
        return 0;
    }
    return 1;
}

int createSHM(key_t key, size_t dim) {
    int idSHM = shmget(key, dim, IPC_CREAT | IPC_EXCL);
    if(idSHM < 0) {
        return 0;
    }
    return idSHM;
}
void *attachSHM(int idSHM) {
    void *addressSHM = shmat(idSHM, NULL, 0);
    return addressSHM;
}
int removeSHM(int idSHM) {
    if(shmctl(idSHM, IPC_RMID, 0) < 0) {
        return 0;
    }
    return 1;
}
void initSHM(int *matrix, const int base, const int higth, const int valInit) {
    int i;
    for(i = 0; i < (base * higth); i++) {
        matrix[i] = valInit;
    }
}