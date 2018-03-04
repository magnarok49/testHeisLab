#include <stdbool.h>
#include "elev.h"

typedef struct order_states {
    bool up;
    bool down;
    bool elev;
} orderStruct;

typedef enum elev_status {
    BEETWEEN_FLOORS = -1,
    FLOOR_ONE,
    FLOOR_TWO,
    FLOOR_THREE,
    FLOOR_FOUR
} elev_status_enum;

/*returns the current unix time. With millisecond precision*/
double get_wall_time(void);

/*removes the first element from the destination queue
* used upon reaching the destination floor*/
void shiftFromQueue();

/*Inserts value element into the destination queue
* BEFORE the given index*/
void insertIntoQueue(int value, int index);

/*Used solely for debugging purposes:
* prints the current destination queue to terminal*/
void printQueue();

/*the workhorse of the queue setup
* adds a given floor to the destination queue
* given that it is not already in the current route.
* this function relies on the orders array,
* which has to updated for addToQueue to work properly*/
void addToQueue(int floorToAdd);

/*drives the elevator to a known state upon startup
* tends downward.
* requires the elevator to be within working range*/
void driveToInitialState(void);

/*sets the elevator running in a given direction
* if there is not an unhandled emergency it sets the dir variable*/
void moveElevator(elev_motor_direction_t direction);

/*Clears everything regarding destination handling,
* IE. empties the queue and zeroes the orders array (and matching lights).
* Used only in emergencyStop.*/
void clearQueueAndOrders();

/*Stops the elevator immediately:
* Clears all existing orders.
* Ignores all other buttons while button is held down.
* If on a floor:
*     opens door and keeps it open for as long at the button is pushed, 
*     as well as 3s after.*/
void emergencyStop();

/*Called concurrently while elevator is on a floor.
* Updates lastFloor.
* Determines if the elevator should stop on the reached floor.
* if so:
*     Extinguishes lights, 
*     shifts from queue (if reached floor was an end destination)
*     zeroes orders array*/
void reachedFloor(int floor);

/*Checks every ordinary button:
* if not already saved in orders (IE. unhandled press registered):
*     lights up the given button
*     updates orders
*     calls addToQueue*/
void pollButtons();

/*Determines the direction to the destination
* uses lastfloor and dir if there is an unhandled emergency present.
* otherwise, uses only lastfloor.
* If no current destination, return DIRN_STOP = 0*/
elev_motor_direction_t getDestinationDir();

/*The main loop for the elevator
* runs here as long as stop button isn't pressed*/
void runElevator();

/*sets the elevator running in the direction given by
* getDestinationDir, provided there is no door timer running
* and the destination queue is nonempty*/
void goToDestination();