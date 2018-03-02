#include "elev.h"
#include "elevatorController.h"
#include "door.h"
#include <stdio.h>

int main() 
{
    if (!elev_init()) 
    {
        printf("Unable to initialize elevator hardware!\n");
        return 1;
    }
    runElevator();
    return 0;
}