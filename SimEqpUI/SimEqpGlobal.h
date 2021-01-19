#ifndef SIMEQPGLOBAL_H
#define SIMEQPGLOBAL_H
#include <iostream>
#include <QtCore>
#include <QDateTime>

class SimEqpGlobal;

class SimEqpGlobal
{
public:

	/*点动参数*/
	static double jog_vel;//mm/s
	static double jog_disp;//mm

	/*限位状态*/
	static bool posLimState;//正 触发为true
	static bool negLimState;//负

	/*力回零状态*/
	static bool forceHomeState; //回零完成为true
	static bool isHoming;//正在回零为true

	/*运动停止状态*/
	static bool isMoving;

	/*当前选择曲线的风速*/
	static int windSpeed;//m/s


	/*当前实时的位移、速度与力*/
	static double rt_disp;//mm
	static double rt_vel;//mm/s
	static double rt_force;//kg
	
	/*实时绘图相关时间 */
	static double intervalTime;
	static QDateTime startTime;
	static QDateTime endTime;

};

enum CurveType
{
	/*曲线类型*/
	Disp,
	Vel,
	Force
};

#endif
