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
        return;
    }
}

void setTimer()
{
    openDoor();
    doorTimer = get_wall_time();
}

bool isTimerFinished()
{
    return doorTimer == 0;
}