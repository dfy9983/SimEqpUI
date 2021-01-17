#pragma once
#pragma execution_character_set("utf-8")
#include "PmacDeviceLib.h"
#include <QObject>
#include <QtCore>
#include <iostream>
#include <QMessageBox>
#include "SimEqpGlobal.h"
#include <QElapsedTimer>
class MyQPmac:public QObject
{
	Q_OBJECT

public:
	MyQPmac();
	~MyQPmac();

	bool creatPmacSelect();
	bool initPmac();
	void enabelMotorServo();
	void enablePLC(int n);
	bool downloadFile(QString strFile);
	void openForceLoop();//力开环，然后才可以运行自己的plc0伺服算法
	void closeLoop();//1:闭环，才能进行速度点动 2:在j+ 连续运动时候使用可以停止运动

	double getMotorDisp();//获取电机位移
	double getMotorVel();
	double getMotorForce();
	bool getPosLimState();//获取正限位状态
	bool getNegLimState();//获取负限位状态
	bool getForceHomeState();//获取力回零状态

	void setJogVel(double vel);//设置点动速度

	void jogDisp(double disp);//距离点动
	void jogPosContinuously();//持续+运动

	void startWinds();//开始运动力曲线
	void stopWinds();//中途停止停止力曲线
	void forceHome();//力回零

	

private:
	PCOMMSERVERLib::PmacDevice *Pmac0;
	int pDeviceNumber;
	bool pbSuccess_select;
	bool pbSuccess_open;
	bool pbSucess_download;
	QString pAnswer;
	bool bAddLF;
	int pstatus;
};

