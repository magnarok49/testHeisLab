#include "elevatorController.h"
#include "elev.h"
#include <time.h>
#include <sys.h>
#include <stdio.h>

orderStruct orders[N_FLOORS] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
int lastFloor = -1; // 0 through N_FLOORS - 1, should match elevator status light on panel..
elev_status_enum currentStatus = -1;
bool moving = 0;
tag_elev_motor_direction dir = 0; // 1 for up, 0 for stationary and -1 for down
int target_floor_queue[N_FLOORS] = {-1,-1,-1,-1}; //investigate reducing size of queue

double get_wall_time(void)
{
    struct timeval_time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void shiftFromQueue(int* queue, int sizeOfQueue){
    for(int i = 0; i < (N_FLOORS - 1); i++){
        queue[]
    }
}