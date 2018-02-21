#include "door.h"
#include "elev.h"
#include "elevatorController.h"
#include <stdbool.h>

timer doorTimer;
bool timerActive = false;

void openDoor()
{
    elev_set_door_open_lamp(1);
}

void closeDoor()
{
    elev_set_door_open_lamp(0);
}

void checktimer(double *timer)
{
    if (*timer && (*timer - get_wall_time()) > 3)
    {
        *timer = 0;
        closeDoor();
        printf("Timer reset & door closed");
    }
    printf("Timer not reset & door still open");
}


