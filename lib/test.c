#include "core.h"

#define CONF_FILE_PATH "../config"

void msleep(int millis) {
    // a current time of milliseconds
    int milliseconds_since = clock() * 1000 / CLOCKS_PER_SEC;

    // needed count milliseconds of return from this timeout
    int end = milliseconds_since + millis;

    // wait while until needed time comes
    do {
        milliseconds_since = clock() * 1000 / CLOCKS_PER_SEC;
    } while (milliseconds_since <= end);
}

int main() {
    int i, num, msec = 0;
    double diff = 0.0, current = 0.0;
    time_t start;
    time_t stop;
    srand(time(NULL));
    struct timespec tim;

    tim.tv_sec = 0;
    tim.tv_nsec = 10000000;

    system("clear");
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

    /*clock_t  currentTime = clock();
    sleep(1);
    clock_t difference = clock() - currentTime;
    msec = difference * 1000 / CLOCKS_PER_SEC;
    printf("%d sec, %d msec\n", msec/1000, msec%1000);*/

    for(i = 0; i < 1; i++) {
        time(&start);
        time(&stop);
        while(difftime(stop, start) < 2) {
            time(&stop);
            //msleep(100);
            nanosleep(&tim, NULL);
            current += 0.01;

            //printf("%lf seconds\n", difftime(stop, start));
        }
        printf("FInish in %lf seconds, time left: %lf\n\n", difftime(stop, start), current);
    }


    return 0;
}
