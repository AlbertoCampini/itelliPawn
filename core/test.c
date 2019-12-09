#include "core.h"

#define CONF_FILE_PATH "../config"

int main() {
    int SO_MIN_HOLD_NSEC = readConfig("SO_NUM_G", HARD_MODE, CONF_FILE_PATH);

    if(SO_MIN_HOLD_NSEC < 0) {
        printLastError();
    } else {
        printf("%d", SO_MIN_HOLD_NSEC);
    }

    return 0;
}
