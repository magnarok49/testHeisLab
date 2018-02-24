#include "elevatorController.h"
#include "elev.h"
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "utilities.h"
#include <stdlib.h>
#include "door.h"


orderStruct orders[N_FLOORS] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
int lastFloor = -1; // 0 through N_FLOORS - 1, should match elevator status light on panel..
elev_status_enum currentStatus = -1;
bool moving = 0;
elev_motor_direction_t dir = 0; // 1 for up, 0 for stationary and -1 for down
int target_floor_queue[N_FLOORS] = {-1,-1,-1,-1}; //investigate reducing size of queue
int target_floor_queue_size = N_FLOORS;



// return seconds
double get_wall_time(void)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void shiftFromQueue()//removes first element from queue
{
    for(int i = 0; i < (target_floor_queue_size - 1); i++)
    {
        target_floor_queue[i] = target_floor_queue[i+1];
    }
    target_floor_queue[target_floor_queue_size-1] = -1;
}

/*
Assumes elevator not on floorToAdd..
Assumes orders has already been updated with requested floor.
*/
void addToQueue(int floorToAdd)
{
    if(floorToAdd > (N_FLOORS - 1) || floorToAdd < 0 || currentStatus == floorToAdd) //floorToAdd has invalid value
    {
        printf("addToQueue received invalid floor...\n");
        return;
    }
    if(target_floor_queue[0] < 0) //queue is empty..
    { 
            target_floor_queue[0] = floorToAdd;
            return;
    }
    if(target_floor_queue[0] == floorToAdd)
    {
        return;
    }

    //Figuring out which direction the new order has.
    int dirRequested = -2; //0 for either, -1 for down etc..
    if(orders[floorToAdd].elev)
    {
        dirRequested = 0;
    } else if (orders[floorToAdd].up || orders[floorToAdd].down)
    {
        dirRequested = orders[floorToAdd].up - orders[floorToAdd].down;
    }
    if(dirRequested < -1)//floorToAdd has invalid value
    {
        printf("addToQueue has been called without corresponding data in orders.\n");
        return;
    }
    //order stuff done


    int signCurrentDir = target_floor_queue[0] - lastFloor; //using sign to describe dir, + is up 
    for(int i = 1; i < target_floor_queue_size; i++)
    {
        if(target_floor_queue[i] < 0)//if no more elements in queue..
        {
            target_floor_queue[i] = floorToAdd;
            printf("Appended floor to queue");
            return;
        }
        if(target_floor_queue[i] == floorToAdd)
        {
            printf("Floor already in queue at appropriate spot..");
            return;
        }
        signCurrentDir = (target_floor_queue[i] - target_floor_queue[i-1])/abs(target_floor_queue[i] - target_floor_queue[i-1]);
        if(signCurrentDir == dirRequested || (!dirRequested))
        {
            if(max(target_floor_queue[i], target_floor_queue[i-1]) > floorToAdd && min(target_floor_queue[i], target_floor_queue[i-1]) < floorToAdd){
                printf("Decided floor already enroute");
                return;
            } 
            else if ((signCurrentDir > 0 && target_floor_queue[i] < floorToAdd)||
                    (signCurrentDir < 0 && target_floor_queue[i] > floorToAdd)){
                    target_floor_queue[i] = floorToAdd;
                    printf("Decided floor is the new extremity for current route");
                    return;
            }
        }
    }
}

void clearQueue(){
    int i = 0;
    while(target_floor_queue[i] > -1 && i < target_floor_queue_size){
        target_floor_queue[i] = -1;
        ++i;
    }
}

// stops elevaotr if it is at desired floor, -1 stops elevator immidiately
void stopElevator(int floor)
{
    assert(floor >= -1);
    assert(floor < N_FLOORS);

    if (floor == -1)
    {
        moveElevator(0);
        printf("Emergency stop activated");
    }
    else if (elev_get_floor_sensor_signal() == floor)
    {
        moveElevator(0);
        elev_set_button_lamp(2,floor,0);
        elev_set_button_lamp(0,floor,0);
        elev_set_button_lamp(1,floor,0);
        printf("Arrived at floor ");
        char c = floor+48;
        printf("%c",c);
        printf(" successfully");
    }
}


void driveToInitialState()
{
    while (elev_get_floor_sensor_signal() < 0)
    {
       
        moveElevator(-1);
       
    }
    moveElevator(0);
    currentStatus = elev_get_floor_sensor_signal();
    lastFloor = currentStatus;
    elev_set_floor_indicator(currentStatus);
   
}

void moveElevator(elev_motor_direction_t direction)
{
    dir = direction;
    moving = 1;
    currentStatus = -1;
    closeDoor();
    elev_set_motor_direction(dir);
}

void emergencyStop()
{
    stopElevator(-1);
    clearQueue();
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
        timer doorTimer = get_wall_time();
        while((doorTimer != 0) && ((doorTimer - get_wall_time()) < 3))
        {
            continue;
        }
        closeDoor();
    }

    
}




void reachedFloor(int floor)
{

    lastFloor = floor;
    if (target_floor_queue[0] == floor)
    {
        shiftFromQueue();

        //code to test if we need to reset button lights here?
        orders[floor].up = 0;
        elev_set_button_lamp(BUTTON_CALL_UP,floor,0);
        orders[floor].down = 0;
        elev_set_button_lamp(BUTTON_CALL_DOWN,floor,0);
        orders[floor].elev = 0;
        elev_set_button_lamp(BUTTON_COMMAND,floor,0);
        currentStatus = floor;


        moveElevator(0);
        setTimer();
    }
    else if ((orders[floor].elev) || (dir == 1 && orders[floor].up) || (dir == -1 && orders[floor].down))
    {
        currentStatus = floor;
        if (orders[floor].elev)
        {
            orders[floor].elev=0;
            elev_set_button_lamp(BUTTON_COMMAND,floor,0);
        }
        if (dir == 1 && orders[floor].up)
        {
            orders[floor].up = 0;
            elev_set_button_lamp(BUTTON_CALL_UP,floor,0);
        }
        else if (dir == -1 && orders[floor].down)
        {
            orders[floor].down = 0;
            elev_set_button_lamp(BUTTON_CALL_DOWN,floor,0);
        }
        moveElevator(0);
        setTimer();
    }
}

void pollButtons(){
    for(int i = 0; i < N_FLOORS; i++){
        if(i < (N_FLOORS - 1) && (!orders[i].up) && elev_get_button_signal(BUTTON_CALL_UP,i)){
            elev_set_button_lamp(BUTTON_CALL_UP, i, 1);
            orders[i].up = 1;
            addToQueue(i);
        }
        if(i > 0 && (!orders[i].down) && elev_get_button_signal(BUTTON_CALL_DOWN,i)){
            elev_set_button_lamp(BUTTON_CALL_DOWN, i, 1);
            orders[i].down = 1;
            addToQueue(i);
        }
        if((!orders[i].elev) && elev_get_button_signal(BUTTON_COMMAND,i)){
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
        if (target_floor_queue[0] > lastFloor)
        {
            moveElevator(1);
        }
        else if (target_floor_queue[0] < lastFloor)
        {
            moveElevator(-1);
        }
    }
    
}



void runElevator()
{
    printf("Flip Obstruction switch to stop elevator and exit program.\n");


    driveToInitialState();

    while (1)
    {
        pollButtons();
        if(elev_get_floor_sensor_signal() > -1)
        {
            reachedFloor(elev_get_floor_sensor_signal());
        }
        goToDestination();
        checkTimer();


        
        if (elev_get_stop_signal())
        {
            emergencyStop();
        }


        // Stop elevator and exit program if the obstruction button is pressed
        if (elev_get_obstruction_signal()) {
            elev_set_motor_direction(DIRN_STOP);
            break;
        }
    }
}