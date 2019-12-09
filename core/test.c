#include "core.h"

int main() {
    int SO_MIN_HOLD_NSEC = readConfig("SO_NUM_G", HARD_MODE);

    if(SO_MIN_HOLD_NSEC < 0) {
        printLastError();
    } else {
        printf("%d", SO_MIN_HOLD_NSEC);
    }

    return 0;
}
