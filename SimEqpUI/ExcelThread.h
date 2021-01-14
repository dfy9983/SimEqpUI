#ifndef EXCELTHREAD_H
#define EXCELTHREAD_H
#pragma execution_character_set("utf-8")
#include "LoadExcelData.h"
#include <QObject>
#include <windows.h>
#include "SimEqpGlobal.h"

class ExcelThread :public QObject
{
	Q_OBJECT

public:
	explicit ExcelThread(QObject *parent = nullptr);
	virtual ~ExcelThread();

public slots:
	void startThread();
	void readData_Thread(CurveType curve, QString fileName, QList<QList<QVariant>> *x_y0);
	void writeData_Thread(CurveType curve, QList<QList<QVariant> > *x_y1, QString fileName);
	void writeExcelIsEmpty_Thread();

signals:
	void writeExcelIsEmpty2main();
	void readExcelFinish2main();
	void writeExcelFinish2main();
	void plotExcelData(CurveType curve);
private:
	LoadExcelData *myExcel;
};

#endif
