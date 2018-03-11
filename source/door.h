#include <stdbool.h>


typedef double timer; //defines a timer of type double

/*opens elevator door (turn elevator door lights on)*/
void openDoor();

/*closes elevator door (turn elevator door lights off)*/
void closeDoor();

/*checks if timer har surpassed 3 seconds, if so timer is set low and door is closed*/
void checkTimer(); 

/*opens door & sets a timer to wall time*/
void setTimer(); 

/*returns a bool depending on timer being active or not
  returns true if timer not active and false if timer active*/
bool isTimerFinished(); 

