#include <stdbool.h>


typedef double timer; //defines a timer of type double

/*returns the current unix time with millisecond precision*/
double getWallTime();

/*opens elevator door (turn elevator door lights on)*/
void openDoor();

/*closes elevator door (turn elevator door lights off)*/
void closeDoor();

/*checks if timer har surpassed 3 seconds, if so timer is set low and door is closed*/
void checkTimer(); 

/*opens door & sets a timer to wall time*/
void setTimer(); 

/*bool that is true as long as there is no timer running.*/
bool isTimerFinished(); 

