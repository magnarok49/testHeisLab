#include "door.h"
#include "elev.h"
#include "elevatorController.h"
#include <stdbool.h>
#include <stdio.h>

timer doorTimer = NULL;

void openDoor()
{
    elev_set_door_open_lamp(1);
}

void closeDoor()
{
    elev_set_door_open_lamp(0);
}

void checktimer()
{
    if (doorTimer && (doorTimer - get_wall_time()) > 3)
    {
        doorTimer = NULL;
        closeDoor();
        printf("Timer reset & door closed");
        return;
    }
    printf("Timer not reset & door still open");
}

void setTimer()
{
    openDoor();
    doorTimer = get_wall_time();
    printf("Timer set & door open");
}

bool timerStatus()
{
    return doorTimer != NULL;
}




