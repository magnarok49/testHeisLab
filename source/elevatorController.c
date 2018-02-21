#include "elevatorController.h"
#include "elev.h"
#include <sys/time.h>
#include <stdio.h>

orderStruct orders[N_FLOORS] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
int lastFloor = -1; // 0 through N_FLOORS - 1, should match elevator status light on panel..
elev_status_enum currentStatus = -1;
bool moving = 0;
elev_motor_direction_t dir = 0; // 1 for up, 0 for stationary and -1 for down
int target_floor_queue[N_FLOORS] = {-1,-1,-1,-1}; //investigate reducing size of queue
int target_floor_queue_size = N_FLOORS;

double get_wall_time(void)
{
    struct timeval_time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void shiftFromQueue(int* queue, int sizeOfQueue){ //removes first element from queue
    for(int i = 0; i < (sizeOfQueue - 1); i++){
        queue[i] = queue[i+1];
    }
}

void insertIntoQueue(int* queue, int sizeOfQueue, int elementToInsert, int index){
    if (index == sizeOfQueue - 1 || queue[sizeOfQueue-1] > -1){
        queue[index] = elementToInsert;
        printf("Size of queue exceeded, losing last element.\n");
        return;
    }
    for(int i = sizeOfQueue - 1; i > index; i--){
        queue[i] = queue[i-1];
    }
    queue[index] = elementToInsert;
    return;
}


/*
Assumes elevator not on floorToAdd..
Assumes orders has already been updated with requested floor.
*/
void addToQueue(int* queue, int sizeOfQueue, int floorToAdd){
    if(floorToAdd > (N_FLOORS - 1) || floorToAdd < 0){//floorToAdd has invalid value
        printf("addToQueue received invalid floor...\n");
        return;
    }
    if(queue[0] < 0){ //queue is empty..
            queue[0] = floorToAdd;
            return;
    }
    if(queue[0] == floorToAdd){
        return;
    }

    //Figuring out which direction the new order has.
    int dirRequested = -2; //0 for either, -1 for down etc..
    if(orders[floorToAdd].elev){
        dirRequested = 0;
    } else if (orders[floorToAdd].up || orders[floorToAdd].down){
        dirRequested = orders[floorToAdd].up - orders[floorToAdd].down;
    }
    if(dirRequested < -1){//floorToAdd has invalid value
        printf("addToQueue has been called without corresponding data in orders.\n");
        return;
    }
    //order stuff done


    int signCurrentDir = queue[0] - lastFloor; //using sign to describe dir, + is up 
    for(int i = 1; i < sizeOfQueue; i++){
        if(queue[i] < 0){ //if no more elements in queue..
            queue[i] = floorToAdd;
            print("Appended floor to queue");
            return;
        }
        if(queue[i] == floorToAdd){
            printf("Floor already in queue at appropriate spot..");
            return;
        }
        signCurrentDir = (queue[i] - queue[i-1])/abs(queue[i] - queue[i-1]);
        if(signCurrentDir == dirRequested || (!dirRequested)){
            if(max(queue[i], queue[i-1]) > floorToAdd && min(queue[i], queue[i-1]) < floorToAdd){
                printf("Decided floor already enroute");
                return;
            } 
            else if ((signCurrentDir > 0 && queue[i] < floorToAdd)||
                    (signCurrentDir < 0 && queue[i] > floorToAdd)){
                    queue[i] = floorToAdd;
                    printf("Decided floor is the new extremity for current route");
                    return;
            }
        }
    }
}