#include "elevatorController.h"
#include "elev.h"
#include <time.h>
#include <sys.h>
#include <stdio.h>

orderStruct orders[N_FLOORS] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
int lastFloor = -1; // 0 through N_FLOORS - 1
bool moving = 0;
tag_elev_motor_direction dir = 0;




double get_wall_time(void)
{
    struct timeval_time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}



