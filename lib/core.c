#include "core.h"

int generateRandom(int to, int from) {
    return (rand() % (from - to + 1)) + to;
}
/*Legge dal file di configurazioni il valore della char *config con la modalità*/
int readConfig(char *config, const char *fPath, int preferedMode) {
    /*int actualMode = -1;*/
    char *line;
    FILE *fConf;

    fConf = fopen(fPath, "r");
    if (fConf == NULL) {
        return -1;
    }

    line = (char *)malloc(sizeof(char) * MAX_LINE_CONF);

    while(fgets(line, sizeof(char) * MAX_LINE_CONF, fConf) != NULL) {
        if(strcmp(line, "") != 0 && strcmp(line, "\n") != 0) {
            /*if(actualMode < 0) {
                actualMode = atoi(strtok(line, "#"));
            } else {
                int modeRead = atoi(strtok(line, "."));
                char *configRead = strtok(NULL, ":");
                int valueRead = atoi(strtok(NULL, ":"));

                if(strcmp(config, configRead) == 0) {
                    if(actualMode == modeRead) {
                        return valueRead;
                    }
                }
            }*/

            int modeRead = atoi(strtok(line, "."));
            char *configRead = strtok(NULL, ":");
            int valueRead = atoi(strtok(NULL, ":"));

            if(strcmp(config, configRead) == 0) {
                if(preferedMode == modeRead) {
                    return valueRead;
                }
            }
        }
    }

    free(line);
    fclose(fConf);
    return -1;
}
void printLastError() {
    printf("%s\n", strerror(errno));
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
int sendMessageToGamer(int idMsg, long msgType, SyncGamer syncGamer) {
    syncGamer.mtype = msgType;
    if(msgsnd(idMsg, &syncGamer, (sizeof(SyncGamer) - sizeof(long)), 0) < 0) {
        return 0;
    }
    return 1;
}
int receiveMessageToMaster(int idMsg, long msgType, void *msg) {
    if(msgrcv(idMsg, msg, (sizeof(SyncGamer) - sizeof(long)), msgType, 0) < 0) {
        return 0;
    }
    return 1;
}
int sendMessageToPawns(int idMsg, long msgType, SyncPawn syncPawn) {
    syncPawn.mtype = msgType;
    if(msgsnd(idMsg, &syncPawn, (sizeof(SyncPawn) - sizeof(long)), 0) < 0) {
        return 0;
    }
    return 1;
}
int receiveMessageToGamer(int idMsg, long msgType, void *msg) {
    if(msgrcv(idMsg, msg, (sizeof(SyncPawn) - sizeof(long)), msgType, 0) < 0) {
        return 0;
    }
    return 1;
}
int sendMessageResultRound(int idMsg, long msgType, ResultRound resultRound) {
    resultRound.mtype = msgType;
    if(msgsnd(idMsg, &resultRound, (sizeof(ResultRound) - sizeof(long)), 0) < 0) {
        return 0;
    }
    return 1;
}
int receiveMessageResultRound(int idMsg, long msgType, void *msg) {
    if(msgrcv(idMsg, msg, (sizeof(ResultRound) - sizeof(long)), msgType, 0) < 0) {
        return 0;
    }
    return 1;
}


int createAndInitSems(key_t semKey, const int nSems, unsigned short valInit) {
    int idSem, i;
    unsigned short *valsInit;
    union semun arg;

    if((idSem = semget(semKey, nSems, IPC_CREAT | IPC_EXCL)) < 0) {
        return 0;
    }

    valsInit = (unsigned short *)malloc(sizeof(unsigned short) * nSems);
    for(i = 0; i < nSems; i++) {
        valsInit[i] = valInit;
    }
    arg.array = valsInit;

    if(semctl(idSem, 0, SETALL, arg) < 0) {
        return 0;
    }

    free(valsInit);
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
int getValueOfSem(int semId, int semNum) {
    int valSem = semctl(semId, semNum, GETVAL);
    return valSem;
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
