#include <stdbool.h>

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
void insertIntoQueue(int* queue, int sizeOfQueue, int elementToInsert, int index);
void addToQueue(int* queue, int sizeOfQueue, int floorToAdd);