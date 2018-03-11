#include <stdbool.h>

typedef enum doorStatus {OPEN = 1,CLOSED = 0}doorStatus_t;

typedef double timer; //defines a timer of type double

void openDoor(); //opens elevator door (turn elevator door lights on)

void closeDoor(); //closes elevator door (turn elevator door lights off)

void checkTimer(); //checks if timer har surpassed 3 seconds, if so timer is set low and door is closed

void setTimer(); //opens door & sets a timer to wall time

bool isTimerFinished(); //returns a bool depending on timer being active or not. Returns true if timer not active and false if timer active.

