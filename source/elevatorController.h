#include <stdbool.h>

typedef struct order_states {
    bool up;
    bool down;
    bool elev;
} orderStruct;

typedef double wall_time;

double get_wall_time(void);

typdef enum elev_status { //-1 for beetween floors, and 0-3 for stationary on a floor.
    BEETWEEN_FLOORS = -1,
    FLOOR_ONE,
    FLOOR_TWO,
    FLOOR_THREE,
    FLOOR_FOUR
} elev_status_enum;



void update_wall_time(void);