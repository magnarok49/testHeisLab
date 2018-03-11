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
    if (doorTimer && (getWallTime() - doorTimer) > 3)
    {
        doorTimer = 0;
        closeDoor();
        return;
    }
}

void setTimer()
{
    openDoor();
    doorTimer = getWallTime();
}

bool isTimerFinished()
{
    return doorTimer == 0;
}