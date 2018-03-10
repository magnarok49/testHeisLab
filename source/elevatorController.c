#include "elevatorController.h"
#include "elev.h"
#include <stdio.h>
#include <string.h>//do we even use this?
#include <assert.h>
#include "utilities.h"
#include <stdlib.h>
#include "door.h"

//private variables for elevatorController
orderStruct orders[N_FLOORS] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};   //should match button lights and thus unhandled requests. (dynamic construction of structs)
elevStatusEnum lastFloor = -1;                                      //0 through N_FLOORS - 1, should match elevator status light on panel. Should only be -1 before init..
elevStatusEnum currentStatus = -1;                                  //contains floor sensor reading
elev_motor_direction_t dir = 0;                                     //1 for up, 0 for stationary and -1 for down
elevStatusEnum targetFloorQueue[N_FLOORS] = {-1,-1,-1,-1};          //End destinations in a fifo-style container
int targetFloorQueueSize = N_FLOORS;                                //could be one less, but avoiding bad_access exceptions if something messes up
bool unhandledEmergency = false;                                    //bool used for keeping track of strange destinations as a result of emergency stops between floors.
int unhandledDirectionalOrder = 0;                                  //saves the requested direction when stopping for a directional request, so it will be prioritized
double positionOnEmergency = -1;                                    //used to remember between-floors location on emergency stops 

//public methods for elevatorController
void shiftFromQueue()
{
    for (int i = 0; i < (targetFloorQueueSize - 1); i++)
    {
        targetFloorQueue[i] = targetFloorQueue[i+1];
    }
    targetFloorQueue[targetFloorQueueSize - 1] = -1;
}

void insertIntoQueue(int value, int index){ //inserts before given index
	for (int i = targetFloorQueueSize - 1; i > index; i--)
    {
        targetFloorQueue[i] = targetFloorQueue[i-1];
    }
    targetFloorQueue[index] = value;
}

void addToQueue(int floorToAdd)
{
    assert(orders[floorToAdd].elev || orders[floorToAdd].up || orders[floorToAdd].down);
    if (targetFloorQueue[0] < 0) //queue is empty..
    { 
        targetFloorQueue[0] = floorToAdd;
        return;
    }
    else if (targetFloorQueue[0] == floorToAdd) //queue is empty..
    { 
        return;
    }

    //Figuring out which direction the new order has.
    int dirRequested = -2; //0 for either, -1 for down etc..
    elev_motor_direction_t signCurrentDir = getDestinationDir();

    if (orders[floorToAdd].elev || floorToAdd == (N_FLOORS - 1) || floorToAdd == 0 || 
        (orders[floorToAdd].up && orders[floorToAdd].down)) //if elevator should stop on given floor as soon as possible
    {
        dirRequested = 0;
    }
    else //if the floor only has one direction ordered.
    {
        dirRequested = orders[floorToAdd].up - orders[floorToAdd].down;
    }

    if (unhandledDirectionalOrder &&
        ((unhandledDirectionalOrder > 0 && floorToAdd > currentStatus)||
        (unhandledDirectionalOrder < 0 && floorToAdd < currentStatus)))//ensures elev stopping for directional order prioritized those first.
    {
        insertIntoQueue(floorToAdd, 0);
        unhandledDirectionalOrder = 0;
        return;
    }


    else if ((signCurrentDir == dirRequested || (!dirRequested)) &&
        ((max(targetFloorQueue[0], lastFloor) > floorToAdd && 
        min(targetFloorQueue[0], lastFloor) < floorToAdd ) ||
        (currentStatus > -1 && floorToAdd == currentStatus))) //if direction matches and floor is enroute
    {
        if((orders[floorToAdd].up && signCurrentDir < 0) || 
        	(orders[floorToAdd].down && signCurrentDir > 0)) //if there is an order placed for the opposite direction 
        {
        	dirRequested = -1*signCurrentDir;
        }
        else
        {
            return;
        }
    } 
    else if ((signCurrentDir > 0 && targetFloorQueue[0] < floorToAdd) ||
            (signCurrentDir < 0 && targetFloorQueue[0] > floorToAdd)) //if floor is the new extremity in the current direction
    {
        if(((orders[targetFloorQueue[0]].up && signCurrentDir < 0) ||
        	(orders[targetFloorQueue[0]].down && signCurrentDir > 0)) &&
        	targetFloorQueue[1] == -1) //no need to insert if elevator is already turning around after the first stop
        {

        	insertIntoQueue(floorToAdd,0);
        	return;
        }
        targetFloorQueue[0] = floorToAdd;
        return;
    }

    for (int i = 1; i < targetFloorQueueSize; i++)
    {
        if (targetFloorQueue[i] < 0)
        {
            targetFloorQueue[i] = floorToAdd;
            return;
        }
        if (targetFloorQueue[i] == floorToAdd)
        {
            return;
        }
        
        if(targetFloorQueue[i] == targetFloorQueue[i-1])//SHOULD NEVER HAPPEN
        {
            signCurrentDir = 0;
        }
        else
        {
            signCurrentDir = (targetFloorQueue[i] - targetFloorQueue[i-1])/abs(targetFloorQueue[i] - targetFloorQueue[i-1]);
        }

        if ((signCurrentDir == dirRequested) &&
            max(targetFloorQueue[i], targetFloorQueue[i-1]) > floorToAdd && 
            min(targetFloorQueue[i], targetFloorQueue[i-1]) < floorToAdd) //floor is enroute
        {
            return;
        } 
        else if ((signCurrentDir >= 0 && targetFloorQueue[i] <= floorToAdd) || //decided if the new floor is further than the existing destination for that direction
                (signCurrentDir <= 0 && targetFloorQueue[i] >= floorToAdd)) //if so, overwrite the existing destination
        {
		if(((orders[targetFloorQueue[i]].up && signCurrentDir < 0 && targetFloorQueue[i] < lastFloor) ||
        	(orders[targetFloorQueue[i]].down && signCurrentDir > 0 && targetFloorQueue[i] > lastFloor)))
        	{

        		insertIntoQueue(floorToAdd,i);
        		return;
        	}
                targetFloorQueue[i] = floorToAdd;
                return;
        }
    }
}

void clearQueueAndOrders()
{
    int i = 0;
    while (targetFloorQueue[i] > -1 && i < targetFloorQueueSize)//clearing queue
    {
        targetFloorQueue[i] = -1;
        ++i;
    }
    for (int j = 0; j < N_FLOORS; j++)//clearing orders array and turning off eventual lights
    {
        if (orders[j].elev)
        {
            orders[j].elev = 0;
            elev_set_button_lamp(BUTTON_COMMAND, j,0);
        }
        if (j < (N_FLOORS - 1) && orders[j].up)
        {
            orders[j].up = 0;
            elev_set_button_lamp(BUTTON_CALL_UP, j,0);
        }
        if (j > 0 && orders[j].down)
        {
            orders[j].down = 0;
            elev_set_button_lamp(BUTTON_CALL_DOWN, j,0);
        }
    }
}

void driveToInitialState()
{
    while (elev_get_floor_sensor_signal() < 0)
    {
        moveElevator(DIRN_DOWN);
    }
    moveElevator(0);
    currentStatus = elev_get_floor_sensor_signal();
    lastFloor = currentStatus;
    elev_set_floor_indicator(currentStatus);
    setTimer();
}

void moveElevator(elev_motor_direction_t direction)
{
    dir = direction;
    elev_set_motor_direction(direction);
    if (direction)
    {
        unhandledDirectionalOrder = 0;
    }
}

void printQueue()
{
	printf("\033[F"); //should go up one line and clear it, untested
    printf("\033[K");
    for(int i = 0; i < targetFloorQueueSize - 1; i++)
	{
		if(targetFloorQueue[i]==-1)
		{
			printf("\n");
			return;
		}
		printf("%d", targetFloorQueue[i]);
		printf(" ");
	}
	printf("\n");
}

void emergencyStop()
{
    elev_set_motor_direction(DIRN_STOP);
    clearQueueAndOrders();
    elev_set_stop_lamp(1);
    if (elev_get_floor_sensor_signal() != -1)
    {
        openDoor();
        dir = 0;
    }
    else 
    {
        if(!unhandledEmergency && dir != 0)
        {
            positionOnEmergency = lastFloor + (dir*0.5);
        }
        unhandledEmergency = 1;
    }
    while (elev_get_stop_signal())
    {
        continue;
    }
    elev_set_stop_lamp(0);
    if (elev_get_floor_sensor_signal() != -1)
    {
        setTimer();
    }
}

void reachedFloor(int floor)
{
    elev_set_floor_indicator(floor);
    lastFloor = floor;
    currentStatus = floor;//SUPERFLUOUS... currentStatus already set to same floor in main loop
    if (targetFloorQueue[0] == floor)
    {
        if(floor < (N_FLOORS - 1) && floor > 0) //makes elevator tend to continueing in a given direction, rather than turning on endpoints.
        {
            unhandledDirectionalOrder = dir;//EMERGENCY STOP NEEDS FIXING, DIR SHOULD ALWAYS REFLECT DRIVING DIRECTION
        }
        
        shiftFromQueue();
        if(floor < 3)
        {
            orders[floor].up = 0;
            elev_set_button_lamp(BUTTON_CALL_UP,floor,0);
        }
        if(floor > 0)
        {
            orders[floor].down = 0;
            elev_set_button_lamp(BUTTON_CALL_DOWN,floor,0);
        }
        orders[floor].elev = 0;
        elev_set_button_lamp(BUTTON_COMMAND,floor,0);
        moveElevator(DIRN_STOP);
        setTimer();
    }
    else if ((orders[floor].elev) || (getDestinationDir() == DIRN_UP && orders[floor].up) || (getDestinationDir() == DIRN_DOWN && orders[floor].down))
    {
        if (orders[floor].elev)
        {
            orders[floor].elev=0;
            elev_set_button_lamp(BUTTON_COMMAND,floor,0);
        }
        if (orders[floor].up && getDestinationDir() == DIRN_UP)
        {
            orders[floor].up = 0;
            elev_set_button_lamp(BUTTON_CALL_UP,floor,0);
        }
        else if (orders[floor].down && getDestinationDir() == DIRN_DOWN)
        {
            orders[floor].down = 0;
            elev_set_button_lamp(BUTTON_CALL_DOWN,floor,0);
        }
        moveElevator(DIRN_STOP);
        setTimer();
    }
}

void pollButtons(){
    for (int i = 0; i < N_FLOORS; i++)
    {
        if (i < (N_FLOORS - 1) && (!orders[i].up) && elev_get_button_signal(BUTTON_CALL_UP,i))
        {
            elev_set_button_lamp(BUTTON_CALL_UP, i, 1);
            orders[i].up = 1;
            addToQueue(i);
        }
        if (i > 0 && (!orders[i].down) && elev_get_button_signal(BUTTON_CALL_DOWN,i))
        {
            elev_set_button_lamp(BUTTON_CALL_DOWN, i, 1);
            orders[i].down = 1;
            addToQueue(i);
        }
        if ((!orders[i].elev) && elev_get_button_signal(BUTTON_COMMAND,i))
        {
            elev_set_button_lamp(BUTTON_COMMAND, i, 1);
            orders[i].elev = 1;
            addToQueue(i);
        }
    }
}

void goToDestination()
{
    if (targetFloorQueue[0] > -1 && isTimerFinished())
    {
        moveElevator(getDestinationDir());
    }
}

elev_motor_direction_t getDestinationDir(){
    if (targetFloorQueue[0] > -1)
    {
        if(unhandledEmergency)
        {
            if(targetFloorQueue[0] > positionOnEmergency)
            {
                return DIRN_UP;
            } 
            else if (targetFloorQueue[0] < positionOnEmergency)
            {
                return DIRN_DOWN;
            }
        }
        else 
        {
            if(targetFloorQueue[0] > lastFloor)
            {
                return DIRN_UP;
            }
            else if (targetFloorQueue[0] < lastFloor)
            {
                return DIRN_DOWN;
            }
        }
    }
    return DIRN_STOP; 
}

void runElevator()
{
    driveToInitialState();
    while (1)
    {
        currentStatus = elev_get_floor_sensor_signal();
        pollButtons();
        printQueue();
        if(currentStatus > -1)
        {
            unhandledEmergency = 0;
            positionOnEmergency = -1;
            reachedFloor(currentStatus);
        }
        goToDestination();
        checkTimer();
        if (elev_get_stop_signal())
        {
            emergencyStop();
        }
    }
}
