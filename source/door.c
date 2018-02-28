#include "door.h"
#include "elev.h"
#include "elevatorController.h"
#include <stdbool.h>
#include <stdio.h>

timer doorTimer = 0;

void openDoor()
{
    elev_set_door_open_lamp(1);
}

void closeDoor()
{
    elev_set_door_open_lamp(0);
}

void checkTimer()
{
    if (doorTimer && (get_wall_time() - doorTimer) > 3)
    {
        doorTimer = 0;
        closeDoor();
        //printf("Timer reset & door closed");
        return;
    }
    //printf("Timer not reset & door still open");
    //double var = get_wall_time()-doorTimer;
    //printf("%lf", var);
    //printf("\n");
    //printf("%lf", get_wall_time());
}

void setTimer()
{
    openDoor();
    doorTimer = get_wall_time();
    //printf("Timer set & door open");
}

bool timerStatus()
{
    return doorTimer == 0;
}




