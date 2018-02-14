#include <stdbool.h>

typedef struct order_states {
    bool up;
    bool down;
    bool elev;
} orderStruct;

typedef double wall_time;

double get_wall_time(void);


void update_wall_time(void);