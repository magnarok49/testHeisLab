#include "elevatorController.h"
#include "elev.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "utilities.h"
#include <stdlib.h>
#include "door.h"

//private variables for elevatorController --------------------------------------

//should match button lights and thus unhandled orders. (dynamic construction of structs)
orderStruct orders[N_FLOORS] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};

//0 through N_FLOORS - 1, should match elevator status light on panel. Should only be -1 before init..
elevStatusEnum lastFloor = -1;

//contains floor sensor reading, 0 through 3 for floors, and -1 for between floors.
elevStatusEnum currentStatus = -1;

//1 for up, 0 for stationary and -1 for down.
elev_motor_direction_t dir = 0;

//End destinations in a fifo-style queue.
elevStatusEnum targetFloorQueue[N_FLOORS] = {-1,-1,-1,-1};

//Size of queue, used for functions like insertIntoQueue and shiftFromQueue.
int targetFloorQueueSize = N_FLOORS;

//bool used for remembering if there is already an unhandled emergency between floors.
bool unhandledEmergency = false;

//Saves the requested direction when stopping for a directional order, so it will be prioritized.
int unhandledDirectionalOrder = 0;

//Used to remember between-floors location on emergency stops. 
double positionOnEmergency = -1;

//public methods for elevatorController --------------------------------------------------------
void shiftFromQueue()
{
    for (int i = 0; i < (targetFloorQueueSize - 1); i++)
    {
        targetFloorQueue[i] = targetFloorQueue[i+1];
    }
    targetFloorQueue[targetFloorQueueSize - 1] = -1;
}

void insertIntoQueue(int value, int index)
{
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
    else if (targetFloorQueue[0] == floorToAdd) //already the first destination
    { 
        return;
    }

    elev_motor_direction_t signCurrentDir = getDestinationDir();

    //Figuring out which direction the new order has. -----------------
    int dirRequested = 0;

    //if elevator should stop on given floor as soon as possible.
    if (orders[floorToAdd].elev || floorToAdd == (N_FLOORS - 1) || floorToAdd == 0 || 
        (orders[floorToAdd].up && orders[floorToAdd].down))
    {
        dirRequested = 0;
    }
    else
    {
        dirRequested = orders[floorToAdd].up - orders[floorToAdd].down;
    }
    //direction decided -----------------------------------------------

    /*ensures that when stopping for a directional order,
    * that direction is priotized afterwards.*/
    if (unhandledDirectionalOrder &&
        ((unhandledDirectionalOrder > 0 && floorToAdd > currentStatus)||
        (unhandledDirectionalOrder < 0 && floorToAdd < currentStatus)))
    {
        insertIntoQueue(floorToAdd, 0);
        unhandledDirectionalOrder = 0;
        return;
    }
    /*if the order is already along the current route,
    * or the elevator is already at the current floor.*/
    else if ((signCurrentDir == dirRequested || (!dirRequested)) &&
        ((max(targetFloorQueue[0], lastFloor) > floorToAdd && 
        min(targetFloorQueue[0], lastFloor) < floorToAdd ) ||
        (currentStatus > -1 && floorToAdd == currentStatus)))
    {
        //if there is an order placed for the opposite direction 
        if((orders[floorToAdd].up && signCurrentDir < 0) || 
        	(orders[floorToAdd].down && signCurrentDir > 0))
        {
        	dirRequested = -1*signCurrentDir;
        }
        else
        {
            return;
        }
    }
    //if floor is further along in the current direction
    else if ((signCurrentDir > 0 && targetFloorQueue[0] < floorToAdd) ||
            (signCurrentDir < 0 && targetFloorQueue[0] > floorToAdd))
    {
        /*if the current destination has an order that doesn't match the current direction
        * this ensures that the elevator will turn around and handle that order afterwards.*/
        if(((orders[targetFloorQueue[0]].up && signCurrentDir < 0) ||
        	(orders[targetFloorQueue[0]].down && signCurrentDir > 0)) &&
        	targetFloorQueue[1] == -1)
        {
        	insertIntoQueue(floorToAdd,0);
        	return;
        }
        targetFloorQueue[0] = floorToAdd;
        return;
    }
    /*iterates through the rest of the queue, 
    * similarly to the code above.*/
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
        /*In normal conditions this never happens, 
        * but remains as a failsafe to ensure zero divisions cannot occur.*/
        if(targetFloorQueue[i] == targetFloorQueue[i-1])
        {
            signCurrentDir = 0;
        }
        else
        {
            signCurrentDir = (targetFloorQueue[i] - targetFloorQueue[i-1])/abs(targetFloorQueue[i] - targetFloorQueue[i-1]);
        }

        if ((signCurrentDir == dirRequested || (!dirRequested)) &&
            max(targetFloorQueue[i], targetFloorQueue[i-1]) > floorToAdd && 
            min(targetFloorQueue[i], targetFloorQueue[i-1]) < floorToAdd)
        {
            /*Ensures both directions get handled if the
            * order still has a stop regardless state*/
            if(!dirRequested)
            {
                if ((signCurrentDir > 0 && orders[floorToAdd].down)||
                    (signCurrentDir < 0 && orders[floorToAdd].up))
                {
                    dirRequested = -1 * signCurrentDir;
                }
                else
                {
                    return;
                }
            }
            else
            {
                return;
            }
        } 
        else if ((signCurrentDir >= 0 && targetFloorQueue[i] <= floorToAdd) ||
                (signCurrentDir <= 0 && targetFloorQueue[i] >= floorToAdd))
        {
		    /*As with before, this test ensures that orders 
            *of opposite direction don't get overwritten*/
            if(((orders[targetFloorQueue[i]].up && signCurrentDir < 0 && targetFloorQueue[i] < lastFloor) ||
        	    (orders[targetFloorQueue[i]].down && signCurrentDir > 0 && targetFloorQueue[i] > lastFloor)))
        	{

        		insertIntoQueue(floorToAdd,i);
        		return;
        	}
            targetFloorQueue[i] = floorToAdd;
            return;
        }
    }//for loop end
}

void clearQueueAndOrders()
{
    int i = 0;
    while (targetFloorQueue[i] > -1 && i < targetFloorQueueSize)//clearing queue
    {
        targetFloorQueue[i] = -1;
        ++i;
    }
    //clearing orders array and turning off eventual lights
    for (int j = 0; j < N_FLOORS; j++)
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
    setTimer(); //this line can be omitted if initializing shouldn't cause doors to open
}

void moveElevator(elev_motor_direction_t direction)
{
    dir = direction;
    elev_set_motor_direction(direction);
    
    /*resets the directional order flag, as it should no longer
    * be prioritized after leaving the floor that set the flag*/
    if (direction)
    {
        unhandledDirectionalOrder = 0;
    }
}

void printQueue()
{
	printf("\033[F"); //moves cursor up one line in the console
    printf("\033[K"); //clears line
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

    //currently on a floor
    if (elev_get_floor_sensor_signal() != -1)
    {
        openDoor();
        dir = 0;
    }
    //currently between floors, calculates intermediate postition.
    else 
    {
        if(!unhandledEmergency && dir != 0)
        {
            positionOnEmergency = lastFloor + (dir*0.5);
        }
        unhandledEmergency = 1;
    }

    //freezes the function until the user releases the button 
    while (elev_get_stop_signal())
    {
        continue;
    }
    elev_set_stop_lamp(0);
    //if elevator is on a floor when emergency button is hit, door remains open 3s after em. button is released
    if (elev_get_floor_sensor_signal() != -1)
    {
        setTimer();
    }
}

void reachedFloor(int floor)
{
    elev_set_floor_indicator(floor);
    lastFloor = floor;
    currentStatus = floor;

    //if stopping for an end destination
    if (targetFloorQueue[0] == floor)
    {
        /*Flag to make the elevator tend to continue along it's current direction.*/
        if(floor < (N_FLOORS - 1) && floor > 0)
        {
            unhandledDirectionalOrder = dir;
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
    //if stopping as an intermediate stop 
    else if ((orders[floor].elev) || 
            (getDestinationDir() == DIRN_UP && orders[floor].up) || 
            (getDestinationDir() == DIRN_DOWN && orders[floor].down))
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

elev_motor_direction_t getDestinationDir()
{
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
    printf("\nElevator is initializing...\n");
    driveToInitialState();
    while (1)
    {
        currentStatus = elev_get_floor_sensor_signal();
        pollButtons();
        printQueue();
        if(currentStatus > -1) //currently on a floor
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
