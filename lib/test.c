#include "core.h"

#define CONF_FILE_PATH "../config"

int main() {
    int i, num, msec = 0;
    double diff = 0.0;
    time_t start;
    time_t stop;
    srand(time(NULL));
    /*int SO_NUM_G = readConfig("SO_N_MOVES", CONF_FILE_PATH);

    if(SO_NUM_G < 0) {
        printLastError();
    } else {
        printf("%d\n", SO_NUM_G);
    }*/

    /*for(i = 0; i < 10; i++) {
        num = generateRandom(5, 40);
        printf("%d", num);
        if(num > 40 || num < 5) {
            printf(" <--");
        }
        printf("\n");
    }*/

    clock_t  currentTime = clock();
    sleep(1);
    clock_t difference = clock() - currentTime;
    msec = difference * 1000 / CLOCKS_PER_SEC;
    printf("%d sec, %d msec\n", msec/1000, msec%1000);

    for(i = 0; i < 3; i++) {
        time(&start);
        time(&stop);
        while(difftime(stop, start) < 3) {
            sleep(1);
            time(&stop);
            printf("%g seconds\n", difftime(stop, start));
        }
        printf("%lf seconds\n\n", difftime(stop, start));
    }


    return 0;
}
