#include "elevatorController.h"
#include "elev.h"
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "utilities.h"
#include <stdlib.h>
#include "door.h"

//private variables for elevatorController
orderStruct orders[N_FLOORS] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};   //should match button lights and thus unhandled requests
int lastFloor = -1;                                                 //0 through N_FLOORS - 1, should match elevator status light on panel. Should only be -1 before init..
elev_status_enum currentStatus = -1;                                //contains floor sensor reading
elev_motor_direction_t dir = 0;                                     //1 for up, 0 for stationary and -1 for down
int target_floor_queue[N_FLOORS] = {-1,-1,-1,-1};                   //investigate reducing size of queue
int target_floor_queue_size = N_FLOORS;                             //arbitrary size, could be halved..
bool unhandledEmergency = false;                                    //bool used for keeping track of strange destinations as a result of emergency stops between floors.

// return unix time in seconds
double get_wall_time(void)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
//removes first element from queue
void shiftFromQueue()
{
    for (int i = 0; i < (target_floor_queue_size - 1); i++)
    {
        target_floor_queue[i] = target_floor_queue[i+1];
    }
    target_floor_queue[target_floor_queue_size - 1] = -1;
}

void insertIntoQueue(int value, int index){ //inserts before given index
	for (int i = target_floor_queue_size - 1; i > index; i--)
    {
        target_floor_queue[i] = target_floor_queue[i-1];
    }
    target_floor_queue[index] = value;
}

//Adds requested floor to queue, if not already enroute in existing destinations, assumes orders array already updated.
void addToQueue(int floorToAdd)
{
    assert(orders[floorToAdd].elev || orders[floorToAdd].up || orders[floorToAdd].down);
    if (target_floor_queue[0] < 0) //queue is empty..
    { 
        target_floor_queue[0] = floorToAdd;
        return;
    }
    else if (target_floor_queue[0] == floorToAdd) //queue is empty..
    { 
        return;
    }


    //Figuring out which direction the new order has.
    int dirRequested = -2; //0 for either, -1 for down etc..
    elev_motor_direction_t signCurrentDir = getDestinationDir();

    if (orders[floorToAdd].elev || floorToAdd == (N_FLOORS - 1) || floorToAdd == 0 || 
        (orders[floorToAdd].up && orders[floorToAdd].down))
    {
        dirRequested = 0;
    }
    else
    {
        dirRequested = orders[floorToAdd].up - orders[floorToAdd].down;
    }

    if ((signCurrentDir == dirRequested || (!dirRequested)) &&
        ((max(target_floor_queue[0], lastFloor) > floorToAdd && 
        min(target_floor_queue[0], lastFloor) < floorToAdd ) || //before ||: tests if the requested floor is within the route
        (currentStatus > -1 && floorToAdd == currentStatus))) //tests if the elevator is already on the requested floor
    {
        if((orders[target_floor_queue[0]].up && signCurrentDir < 0) ||
        	(orders[target_floor_queue[0]].down && signCurrentDir > 0))
        {
        	dirRequested = -1*signCurrentDir;
        }
        else
        {
            return;
        }
    } 
    else if ((signCurrentDir > 0 && target_floor_queue[0] < floorToAdd) ||
                (signCurrentDir < 0 && target_floor_queue[0] > floorToAdd))
    {
        if(((orders[target_floor_queue[0]].up && signCurrentDir < 0) ||
        	(orders[target_floor_queue[0]].down && signCurrentDir > 0)) &&
        	target_floor_queue[1] == -1) //no need to insert if elevator is already turning around after the first stop
        {
        	insertIntoQueue(floorToAdd,0);
        	return;
        }
        target_floor_queue[0] = floorToAdd;
        return;
    }

    for (int i = 1; i < target_floor_queue_size; i++)
    {
        if (target_floor_queue[i] < 0)//if no more elements in queue..
        {
            target_floor_queue[i] = floorToAdd;
            return;
        }
        if (target_floor_queue[i] == floorToAdd)
        {
            return;
        }
        
        if(target_floor_queue[i] == target_floor_queue[i-1])
        {
            signCurrentDir = 0;
        }
        else 
        {
            signCurrentDir = (target_floor_queue[i] - target_floor_queue[i-1])/abs(target_floor_queue[i] - target_floor_queue[i-1]);
        }

        if ((signCurrentDir == dirRequested || (!dirRequested)) &&
            max(target_floor_queue[i], target_floor_queue[i-1]) > floorToAdd && 
            min(target_floor_queue[i], target_floor_queue[i-1]) < floorToAdd)
        {
            return;
        } 
        else if ((signCurrentDir >= 0 && target_floor_queue[i] <= floorToAdd) || //decided if the new floor is "more" extreme than the existing destination for that direction
                (signCurrentDir <= 0 && target_floor_queue[i] >= floorToAdd)) //if so, overwrite the existing destination
        {
                target_floor_queue[i] = floorToAdd;
                return;
        }
    }
}

void clearQueueAndOrders()
{
    int i = 0;
    while (target_floor_queue[i] > -1 && i < target_floor_queue_size)
    {
        target_floor_queue[i] = -1;
        ++i;
    }
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
    setTimer();
}

void moveElevator(elev_motor_direction_t direction)
{
    if(!unhandledEmergency) //direction needs to stay constant while an emergency is being handled
    {
    	dir = direction;
    }
    //dir = direction; // removed closeDoors() call from here, shouldn't be necessary..
    elev_set_motor_direction(direction);
}

void printQueue()
{
	for(int i = 0; i < target_floor_queue_size - 1; i++)
	{
		if(target_floor_queue[i]==-1)
		{
			printf("\n");
			return;
		}
		printf("%d", target_floor_queue[i]);
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
    assert(floor >= 0);
    elev_set_floor_indicator(floor);
    lastFloor = floor;
    currentStatus = floor;
    if (target_floor_queue[0] == floor)
    {
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
    if (target_floor_queue[0] > -1 && timerStatus())
    {
        moveElevator(getDestinationDir());
    }
}

elev_motor_direction_t getDestinationDir(){
    if (target_floor_queue[0] > -1)
    {
        if(unhandledEmergency)
        {
            if ((target_floor_queue[0] > lastFloor && dir > 0 ) ||
                (target_floor_queue[0] == lastFloor && dir < 0 ))
            {
                return DIRN_UP;
            }
            else
            {
                return DIRN_DOWN;
            }
        }
        else 
        {
            if(target_floor_queue[0] > lastFloor)
            {
                return DIRN_UP;
            }
            else if (target_floor_queue[0] < lastFloor)
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
