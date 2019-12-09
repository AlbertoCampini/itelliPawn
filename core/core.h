#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EASY_MODE 0
#define HARD_MODE 1
#define MAXLINECONF 100

extern char *readConfig(char *config, int mode);
char *readConfig(char *config, int mode) {
    FILE *fConf = fopen("../config", "r");
    if (fConf == NULL) {
        return NULL;
    }
    char line[MAXLINECONF];
    while(fgets(line, sizeof(line), fConf) != NULL) {
        if(strcmp(line, "") != 0) {
            printf("Line: %s\n", line);
            //char *mode = strtok(line, ".");
            //printf("%s\n", mode);
            int mode = atoi(strtok(line, "."));
            printf("%d\n", mode);
        }

    }
    fclose(fConf);
    return config;
}
