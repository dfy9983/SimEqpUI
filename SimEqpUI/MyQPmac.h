#pragma once
#pragma execution_character_set("utf-8")
#include "PmacDeviceLib.h"
#include <QObject>
#include <QtCore>
#include <iostream>
#include <QMessageBox>
#include "SimEqpGlobal.h"
class MyQPmac:public QObject
{
	Q_OBJECT

public:
	MyQPmac();
	~MyQPmac();

	bool creatPmacSelect();
	void enabelMotorServo();
	double getMotorDisp();
	double getMotorVel();
	double getMotorForce();
	bool getLimState();
	bool getForceHomeState();
	void setJogVel(double vel);
	void jogDisp(double disp);


private:
	PCOMMSERVERLib::PmacDevice *Pmac0;
	int pDeviceNumber;
	bool pbSuccess_select;
	bool pbSuccess_open;
	QString pAnswer;
	bool bAddLF;
	int pstatus;
};

