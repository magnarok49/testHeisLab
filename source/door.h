#include <stdbool.h>

typedef enum doorStatus{OPEN = 1,CLOSED = 0};

void openDoor();
void closeDoor();

typedef double timer;
bool timerActive = false;

void checkTimer(double *timer);


void emergencyStop();