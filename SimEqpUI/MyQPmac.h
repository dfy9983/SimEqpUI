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

	double getMotorDisp();
	double getMotorVel();
	double getMotorForce();
	bool getLimState();
	bool getForceHomeState();

	void setJogVel(double vel);
	void jogDisp(double disp);


	void startWinds();//开始运动力曲线
	void stopWinds();//中途停止停止力曲线

	

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

