#include "ExcelThread.h"
ExcelThread::ExcelThread(QObject *parent)
{
}
ExcelThread::~ExcelThread()
{
	delete myExcel;
	/*用于释放掉初始化的com*/
	OleUninitialize();

}

void ExcelThread::readData_Thread(CurveType curve, QString fileName, QList<QList<QVariant>>* x_y0)
{
	qDebug() << "sig_ReadDataDis -> readData_Thread:" << QThread::currentThreadId();
	myExcel->readExcelFast(fileName, *x_y0);
	emit readExcelFinish2main();
	qDebug() << "readData_Thread: readExcelFast finished !"<<"curve:"<<curve;
	emit plotExcelData(curve);
}

void ExcelThread::writeData_Thread(CurveType curve, QList<QList<QVariant>>* x_y1, QString fileName)
{
	qDebug() << "WriteDataDis_Thread 进入工作线程:" << QThread::currentThreadId();
	myExcel->writeExcelFast(*x_y1, fileName);
	emit writeExcelFinish2main();
	qDebug() << "writeExcelFast finished !"<<"curve:"<<curve;
}

void ExcelThread::writeExcelIsEmpty_Thread()
{
	emit writeExcelIsEmpty2main();
}

void ExcelThread::startThread()
{
	/*在线程开始的时候初始化COM库*/
	HRESULT r = OleInitialize(0);
	if (r != S_OK && r != S_FALSE)
	{
		qWarning("Qt:初始化Ole失败（error %x）", (unsigned int)r);
	}
	qDebug() << "ExcelThread::startThread:" << QThread::currentThreadId();
	myExcel = new LoadExcelData;
	//向GUI线程发送 保存数据为空 信号
	connect(myExcel, &LoadExcelData::writeExcelisEmpty, this, &ExcelThread::writeExcelIsEmpty_Thread);
}