#include "door.h"
#include "elev.h"
#include "elevatorController.h"



void openDoor()
{
    elev_set_door_open_lamp(1);
}

void closeDoor()
{
    elev_set_door_open_lamp(0);
}

