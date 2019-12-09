#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define EASY_MODE 0
#define HARD_MODE 1
#define MAX_LINE_CONF 100

extern const int readConfig(char *config, int mode);
extern void printLastError();

//Legge dal file di configurazioni il valore della char *config con la modalità
const int readConfig(char *config, int mode) {
    FILE *fConf = fopen("../config", "r");
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

//Stampa in output l'ultimo errore di Core
void printLastError() {
    printf("%s", strerror(errno));
}
