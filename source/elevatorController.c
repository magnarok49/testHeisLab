#include "elevatorController.h"
#include "elev.h"

orderStruct orders[N_FLOORS] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
int lastFloor = -1; // 0 through N_FLOORS - 1
bool driving = 0;
