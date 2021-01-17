#include "MyQPmac.h"

MyQPmac::MyQPmac()
{
	Pmac0 = new PCOMMSERVERLib::PmacDevice();
	this->pDeviceNumber = 0;
	this->pbSuccess_open = false;
	this->pbSuccess_select = false;
	this->pbSucess_download = false;
	this->pAnswer = "";
	this->bAddLF = true;
	this->pstatus = 0;

}
MyQPmac::~MyQPmac()
{
	if (pbSuccess_open)
	{
		Pmac0->Close(pDeviceNumber);
		qDebug() << "Pmac0->Close";
	}
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
		QMessageBox::information(NULL, "提示", "连接成功,请初始化Pmac！");
		return  true;
	}
	else
	{
		qDebug() << "open failed";
		QMessageBox::information(NULL, "提示", "连接失败，检查是否以管理员身份运行！");
		return false;
	}
}

bool MyQPmac::initPmac()
{

	QString strFile_PmacProg = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/PmacProg";
	QString strInitFile = strFile_PmacProg + "/0_initializer.pmc";
	bool downloadState_init = downloadFile(strInitFile);//下载初始化程序
	qDebug() << "downloadState_init:" << strInitFile << "state: " << downloadState_init;
	return  downloadState_init;
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

bool MyQPmac::getPosLimState()
{
	bool isTriggered;
	Pmac0->GetResponse(pDeviceNumber, "M131", pAnswer);//获取正限位状态
	if (pAnswer.toInt())
	{
		isTriggered = true;
	}
	else
	{
		isTriggered = false;
	}
	return isTriggered;
}

bool MyQPmac::getNegLimState()
{
	bool isTriggered;
	Pmac0->GetResponse(pDeviceNumber, "M132", pAnswer);//获取负限位状态
	if (pAnswer.toInt())
	{
		isTriggered = true;
	}
	else
	{
		isTriggered = false;
	}
	return isTriggered;
}

bool MyQPmac::getForceHomeState()
{
	bool isHome = false;
	Pmac0->GetResponse(pDeviceNumber, "p25", pAnswer);//获取力回零状态
	if (pAnswer.toInt())
	{
		isHome = true;
	}
	else
	{
		isHome = false;
	}
	return isHome;
}

bool MyQPmac::getMoveState()
{
	bool isMoving = false;
	Pmac0->GetResponse(pDeviceNumber, "p26", pAnswer);//获取运动状态
	if (pAnswer.toInt())
	{
		isMoving = true;
	}
	else
	{
		isMoving = false;
	}
	return isMoving;
}

void MyQPmac::setJogVel(double vel)
{
	QString strVel = QString::number(vel/1000*512,'f',4);//速度参数换算
	QString strCommand = "I122=" + strVel;
	Pmac0->GetResponse(pDeviceNumber, strCommand, pAnswer);
	qDebug() << "setJogVel:" << strCommand<<":"<< pAnswer;
}

void MyQPmac::jogDisp(double disp)
{
	//换算成脉冲
	int jog_disp_cts = disp * 512;//位移转为脉冲
	QString strCts = QString::number(jog_disp_cts);
	QString strCommand = "j^" + strCts;
	Pmac0->GetResponse(pDeviceNumber, strCommand, pAnswer);//获取速度mm/s
	qDebug() << "jogDisp:" << strCommand << ":" << pAnswer;
}

void MyQPmac::jogPosContinuously()
{
	Pmac0->GetResponse(pDeviceNumber, "j+", pAnswer);//持续正向运动
	qDebug() << "jogPosContinuously 'j+': " << pAnswer;

}

void MyQPmac::jogNegContinuously()
{
	Pmac0->GetResponse(pDeviceNumber, "j-", pAnswer);//持续负向运动
	qDebug() << "jogPosContinuously 'j-': " << pAnswer;

}

void MyQPmac::startWinds()
{ 
	qDebug() << "startWinds";
	enablePLC(4);//开始运动
}

void MyQPmac::stopMove()
{
	enablePLC(5);//中途停止运动
	qDebug() << "stopMove" ;
}

void MyQPmac::forceHome()
{
	enablePLC(3);//力回零Plc
}

void MyQPmac::setforceHomeState(bool isHome)
{
	QString strishome;
	if (isHome)
	{
		strishome = "1";
	}
	else
	{
		strishome = "0";
	}
	QString strCommand = "p25=" + strishome;
	Pmac0->GetResponse(pDeviceNumber, strCommand, pAnswer);
	qDebug() << "setforceHomeState:" << strishome;
}

void MyQPmac::enablePLC(int plcnum)
{
	QString plcNum = QString::number(plcnum);
	QString strCommand = "enable plc " + plcNum;
	Pmac0->GetResponse(pDeviceNumber, strCommand, pAnswer);
	qDebug() << strCommand << ":"<<pAnswer;
}

bool MyQPmac::downloadFile(QString strFile)
{
	pbSucess_download = false;
	Pmac0->Download(pDeviceNumber, strFile, true , true, true, true, pbSucess_download);
	if (pbSucess_download)
	{
		qDebug() << "下载文件:" << strFile << "成功";
		//经过过查阅手册，只是成功开始现在线程，不代表下载成功，需查看日志文件
	}
	return pbSucess_download;
}

void MyQPmac::openForceLoop()
{
	Pmac0->GetResponse(pDeviceNumber, "#1o0", pAnswer);
	qDebug() << "openForceLoop '#1o0':" << pAnswer;
}

void MyQPmac::closeLoop()
{
	Pmac0->GetResponse(pDeviceNumber, "#1j/", pAnswer);
	qDebug() << "closeLoop '#1j/':" << pAnswer;

}



