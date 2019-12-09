#include "core.h"

int main() {
    if(readConfig("Prova", 0) == NULL) {
        printf("\nerrore");
    }
    return 0;
}
