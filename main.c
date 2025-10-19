#include "lib/TP1.h"
#include <stdio.h>

int main(void) {
    // randomize seed to ensure any numbers generated randomly in this program are actually random
    srand(time(NULL));
    
    mainMenu();
    return 0;
}