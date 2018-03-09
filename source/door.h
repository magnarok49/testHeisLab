#include <stdbool.h>

typedef enum doorStatus {OPEN = 1,CLOSED = 0}doorStatus_t;

void openDoor();
void closeDoor();

typedef double timer;

void checkTimer();

void setTimer();

bool isTimerFinished();

