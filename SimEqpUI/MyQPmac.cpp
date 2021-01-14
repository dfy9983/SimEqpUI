#include "MyQPmac.h"

MyQPmac::MyQPmac()
{
	Pmac0 = new PCOMMSERVERLib::PmacDevice();
	this->pDeviceNumber = 0;
	this->pbSuccess_open = false;
	this->pbSuccess_select = false;
	this->pAnswer = "";
	this->bAddLF = true;
	this->pstatus = 0;

}
MyQPmac::~MyQPmac()
{

	if (pbSuccess_open) Pmac0->Close(pDeviceNumber);
	delete Pmac0;
}

bool MyQPmac::creatPmacSelect()
{
	qDebug() << "Pmac created , please select device";
	pbSuccess_open = false;
	pbSuccess_select = false;
	Pmac0->SelectDevice(NULL, pDeviceNumber, pbSuccess_select);
	Pmac0->Open(pDeviceNumber, pbSuccess_open);
	if (pbSuccess_open)
	{
		qDebug() << "open success";
		QMessageBox::information(NULL, "提示", "连接成功");
		return  true;
	}
	else
	{
		qDebug() << "open failed";
		QMessageBox::information(NULL, "提示", "连接失败");
		return false;
	}
}

void MyQPmac::enabelMotorServo()
{
}

double MyQPmac::getMotorDisp()
{
	Pmac0->GetResponse(pDeviceNumber, "M162", pAnswer);//获取位移mm
	double disp = pAnswer.left(pAnswer.length() - 1).toDouble() / 3072 / 8192 * 16;//位移换算
	return disp;
}

double MyQPmac::getMotorVel()
{
	Pmac0->GetResponse(pDeviceNumber, "M174", pAnswer);//获取速度mm/s
	double vel = pAnswer.left(pAnswer.length() - 1).toDouble() *2451/ 3072 / 8192 * 16;//速度换算
	return vel;
}

double MyQPmac::getMotorForce()
{
	Pmac0->GetResponse(pDeviceNumber, "P15", pAnswer);//获取速度mm/s
	double force = pAnswer.left(pAnswer.length() - 1).toDouble() ;//速度换算
	return force;
}

bool MyQPmac::getLimState()
{
	return false;
}

bool MyQPmac::getForceHomeState()
{

	return false;
}

void MyQPmac::setJogVel(double vel)
{
	QString strVel = QString::number(vel/1000*512,'f',4);//速度参数换算
	QString strCommand = "I122=" + strVel;
	qDebug() << "setJogVel:" << strCommand;
	Pmac0->GetResponse(pDeviceNumber, strCommand, pAnswer);
}

void MyQPmac::jogDisp(double disp)
{
	//换算成脉冲
	int jog_disp_cts = disp * 512;
	QString strCts = QString::number(jog_disp_cts);//速度参数换算
	QString strCommand = "j^" + strCts;
	Pmac0->GetResponse(pDeviceNumber, strCommand, pAnswer);//获取速度mm/s

}

