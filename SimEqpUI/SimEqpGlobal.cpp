#include "SimEqpGlobal.h"

//初始化静态变量
double SimEqpGlobal::rt_disp = 0;
double SimEqpGlobal::rt_vel = 0;
double SimEqpGlobal::rt_force = 0;
QDateTime SimEqpGlobal::startTime;
QDateTime SimEqpGlobal::endTime;
double SimEqpGlobal::intervalTime = 0;

double SimEqpGlobal::jog_vel = 0;
double SimEqpGlobal::jog_disp = 0;

int SimEqpGlobal::windSpeed = 0;

bool SimEqpGlobal::posLimState = false;
bool SimEqpGlobal::negLimState = false;
bool SimEqpGlobal::forceHomeState = false;
