#include "door.h"
#include "elev.h"
#include "elevatorController.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>

timer doorTimer = 0;

double getWallTime()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

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