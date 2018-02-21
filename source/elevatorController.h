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

void shiftFromQueue(int* queue, int sizeOfQueue);
void addToQueue(int* queue, int sizeOfQueue, int floorToAdd);

// stops elevaotr if it is at desired floor, -1 stops elevator immidiately
void stopElevator(int floor);

void driveToInitialState(void);

void moveElevator(elev_motor_direction_t direction);
