#ifndef LOADEXCELDATA_H
#define LOADEXCELDATA_H
#pragma execution_character_set("utf-8")
#include <QtCore>
#include <iostream>
#include <QAxObject>
#include <QDir>
#include <QObject>
#include <QMetaType>
#include <QVector>
#include <QVariant>

class LoadExcelData;

class LoadExcelData :public QObject
{
	Q_OBJECT
public:
	explicit LoadExcelData();
	virtual ~LoadExcelData();

	/*******************类型转换*******************/
		/*把QVariant转为QList<QList<QVariant>>,用于快速读出*/
	static void castVariant2ListListVariant(const QVariant &var, QList<QList<QVariant> > &x_y);

	/*把QVariant转为QVector<double>,用于快速读出*/
	static void castVariant2VectorAndVector(const QVariant &var, QVector<double> &x, QVector<double> &y);

	/*把QList<QList<QVariant>>转为QVariant,用于快速写入*/
	static void castListListVariant2Variant(const QList<QList<QVariant> > &x_y, QVariant &res);

	/*把QList<QList<QVariant>>转为QVariant以及QVector<double>,用于快速写入*/
	static void castListListVariant2VectorAndVector(const QList<QList<QVariant> > &x_y, QVector<double> &x, QVector<double> &y);

	/*把列数转换为excel的字母列号*/
	static void convert2ColName(int data, QString &res);

	/*数字转换为26字母*/
	static QString to26AlphabetString(int data);
	/**********************************************/

		/*读取*/
	void readExcelFast(QString fileName, QList<QList<QVariant> > &x_y);//快速读取函数

	/*写入*/
	void writeExcelFast(const QList<QList<QVariant> > &x_y, QString fileName);//快速写入
	void setExcelCell(QAxObject *worksheet, int row, int column, QString text);//按单元写入
signals:
	void writeExcelisEmpty();
private:
	QAxObject * excel;
	QAxObject * workbooks;
	QAxObject * workbook;
	QAxObject * worksheets;
	QAxObject * worksheet;
	QAxObject * usedrange_Read;//读取数据矩形区域
	QAxObject * usedrange_Write;//写入数据矩形区域
	QAxObject * rows;//行数
	QAxObject * columns;//列数
	int WorkSheetCount;//Excel文件中表的个数
	int RowsCount;//行总数
	int ColumnsCount;//列总数
	int StartRow;//数据的起始行
	int StartColumn;//数据的起始列
	QVariant var;
};

#endif // LOADDATA_H
