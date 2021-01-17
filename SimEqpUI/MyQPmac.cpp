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
		QMessageBox::information(NULL, "��ʾ", "���ӳɹ�,���ʼ��Pmac��");
		return  true;
	}
	else
	{
		qDebug() << "open failed";
		QMessageBox::information(NULL, "��ʾ", "����ʧ�ܣ�����Ƿ��Թ���Ա������У�");
		return false;
	}
}

bool MyQPmac::initPmac()
{

	QString strFile_PmacProg = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/PmacProg";
	QString strInitFile = strFile_PmacProg + "/0_initializer.pmc";
	bool downloadState_init = downloadFile(strInitFile);//���س�ʼ������
	qDebug() << "downloadState_init:" << strInitFile << "state: " << downloadState_init;
	return  downloadState_init;
}

void MyQPmac::enabelMotorServo()
{
}

double MyQPmac::getMotorDisp()
{
	Pmac0->GetResponse(pDeviceNumber, "M162", pAnswer);//��ȡλ��mm
	double disp = pAnswer.left(pAnswer.length() - 1).toDouble() / 3072 / 8192 * 16;//λ�ƻ���
	return disp;
}

double MyQPmac::getMotorVel()
{
	Pmac0->GetResponse(pDeviceNumber, "M174", pAnswer);//��ȡ�ٶ�mm/s
	double vel = pAnswer.left(pAnswer.length() - 1).toDouble() *2451/ 3072 / 8192 * 16;//�ٶȻ���
	return vel;
}

double MyQPmac::getMotorForce()
{
	Pmac0->GetResponse(pDeviceNumber, "P15", pAnswer);//��ȡ�ٶ�mm/s
	double force = pAnswer.left(pAnswer.length() - 1).toDouble() ;//�ٶȻ���
	return force;
}

bool MyQPmac::getPosLimState()
{
	bool isTriggered;
	Pmac0->GetResponse(pDeviceNumber, "M131", pAnswer);//��ȡ����λ״̬
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
	Pmac0->GetResponse(pDeviceNumber, "M132", pAnswer);//��ȡ����λ״̬
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
	Pmac0->GetResponse(pDeviceNumber, "p25", pAnswer);//��ȡ������״̬
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
	Pmac0->GetResponse(pDeviceNumber, "p26", pAnswer);//��ȡ�˶�״̬
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
	QString strVel = QString::number(vel/1000*512,'f',4);//�ٶȲ�������
	QString strCommand = "I122=" + strVel;
	Pmac0->GetResponse(pDeviceNumber, strCommand, pAnswer);
	qDebug() << "setJogVel:" << strCommand<<":"<< pAnswer;
}

void MyQPmac::jogDisp(double disp)
{
	//���������
	int jog_disp_cts = disp * 512;//λ��תΪ����
	QString strCts = QString::number(jog_disp_cts);
	QString strCommand = "j^" + strCts;
	Pmac0->GetResponse(pDeviceNumber, strCommand, pAnswer);//��ȡ�ٶ�mm/s
	qDebug() << "jogDisp:" << strCommand << ":" << pAnswer;
}

void MyQPmac::jogPosContinuously()
{
	Pmac0->GetResponse(pDeviceNumber, "j+", pAnswer);//���������˶�
	qDebug() << "jogPosContinuously 'j+': " << pAnswer;

}

void MyQPmac::jogNegContinuously()
{
	Pmac0->GetResponse(pDeviceNumber, "j-", pAnswer);//���������˶�
	qDebug() << "jogPosContinuously 'j-': " << pAnswer;

}

void MyQPmac::startWinds()
{ 
	qDebug() << "startWinds";
	enablePLC(4);//��ʼ�˶�
}

void MyQPmac::stopMove()
{
	enablePLC(5);//��;ֹͣ�˶�
	qDebug() << "stopMove" ;
}

void MyQPmac::forceHome()
{
	enablePLC(3);//������Plc
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
		qDebug() << "�����ļ�:" << strFile << "�ɹ�";
		//�����������ֲᣬֻ�ǳɹ���ʼ�����̣߳����������سɹ�����鿴��־�ļ�
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



