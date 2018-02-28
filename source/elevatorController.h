#include <stdbool.h>
#include "elev.h"

typedef struct order_states {
    bool up;
    bool down;
    bool elev;
} orderStruct;




double get_wall_time(void);

typedef enum elev_status {
    BEETWEEN_FLOORS = -1,
    FLOOR_ONE,
    FLOOR_TWO,
    FLOOR_THREE,
    FLOOR_FOUR
} elev_status_enum;

void shiftFromQueue();
void addToQueue(int floorToAdd);

void driveToInitialState(void);

void moveElevator(elev_motor_direction_t direction);

void clearQueueAndOrders();

void emergencyStop();

void reachedFloor(int floor);

void pollButtons();

void runElevator();

void goToDestination();
