#include "elevatorController.h"
#include "elev.h"
#include <sys/time.h>
#include <stdio.h>


orderStruct orders[N_FLOORS] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
int lastFloor = -1; // 0 through N_FLOORS - 1, should match elevator status light on panel..
elev_status_enum currentStatus = -1;
bool moving = 0;
tag_elev_motor_direction dir = 0;






// return seconds
double get_wall_time(void)
{
    struct timeval_time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}



// stops elevaotr if it is at desired floor, -1 stops elevator immidiately
void stopElevator(int floor)
{
    assert(floor >= -1);
    assert(floor < N_FLOORS);

    if (floor == -1)
    {
        elev_set_motor_direction(0);
        printf("Emergency stop activated");
    }
    else if (elev_get_floor_sensor_signal() == floor)
    {
        elev_set_motor_direction(0);
        elev_set_button_lamp(2,floor,0);
        elev_set_button_lamp(0,floor,0);
        elev_set_button_lamp(1,floor,0);
        printf("Arrived at floor " + string(floor) + string("successfully"));
    }
}


void driveToInitialState()
{
   while (elev_get_floor_sensor_signal == -1)
   {
       
       moving = 1;
       dir = -1;
       elev_set_motor_direction(dir);
       
   }
   elev_set_motor_direction(0);
   currentStatus = elev_get_floor_sensor_signal();
   lastFloor = elev_get_floor_sensor_signal();
   moving = 0;
   
}

void moveElevator(tag_elev_motor_direction direction)
{
    dir = direction;
    moving = 1;
    currentStatus = -1;
    elev_set_motor_direction(dir);
}