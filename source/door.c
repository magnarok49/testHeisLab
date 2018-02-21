#include "door.h"
#include "elev.h"
#include "elevatorController.h"
#include <stdbool.h>
#include <stdio.h>

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



void emergencyStop()
{
    stopElevator(-1);
    elev_set_stop_lamp(1);
    if (elev_get_floor_sensor_signal() != -1)
    {
        openDoor();
       
    }
    while (elev_get_stop_signal())
    {
        continue;
    }
    elev_set_stop_lamp(0);

    if (elev_get_floor_sensor_signal() != -1)
    {
        doorTimer = get_wall_time();
        while((doorTimer != 0) && ((doorTimer - get_wall_time()) < 3))
        {
            continue;
        }
        closeDoor();
    }

    
}
