#include "LoadExcelData.h"
LoadExcelData::LoadExcelData()
{
	excel = new QAxObject("Excel.Application");//加载Excel驱动
	excel->setProperty("Visible", false); //不显示Excel界面，如果为true会看到启动的Excel界面
	workbooks = excel->querySubObject("WorkBooks");
}
LoadExcelData::~LoadExcelData()
{
	delete excel;
	excel = nullptr;
}

void LoadExcelData::castVariant2ListListVariant(const QVariant & var, QList<QList<QVariant>>& x_y)
{
	QVariantList varRows;
	varRows = var.toList();
	if (varRows.isEmpty()) {
		return;
	}
	const int rowCount = varRows.size();
	QVariantList rowData;
	for (int i = 0; i < rowCount; ++i) {
		rowData = varRows[i].toList();
		x_y.push_back(rowData);
	}
}

void LoadExcelData::castVariant2VectorAndVector(const QVariant & var, QVector<double>& x, QVector<double>& y)
{
	QVariantList varRows;
	varRows = var.toList();
	if (varRows.isEmpty()) {
		return;
	}
	const int rowCount = varRows.size();
	QVariantList rowData;
	for (int i = 0; i < rowCount; ++i) {
		rowData = varRows[i].toList();
		x.push_back(rowData.value(0).toDouble());
		y.push_back(rowData.value(1).toDouble());
	}
}

void LoadExcelData::castListListVariant2Variant(const QList<QList<QVariant>>& x_y, QVariant & res)
{
	QVariantList vars;
	const int rowCount = x_y.size();
	for (int i = 0; i < rowCount; ++i) {
		vars.append(QVariant(x_y[i]));
	}
	res = QVariant(vars);
}

void LoadExcelData::castListListVariant2VectorAndVector(const QList<QList<QVariant>>& x_y, QVector<double> &x, QVector<double> &y)
{
	QVariant tempData;
	castListListVariant2Variant(x_y, tempData);
	castVariant2VectorAndVector(tempData, x, y);
}
/****************************************************************
** Function name ：convert2ColName
** Description   : 把列数转换为excel的字母列号,如1->A 26->Z 27 AA
** Parameter     ：data>0
** Return        ：
*****************************************************************/
void LoadExcelData::convert2ColName(int data, QString & res)
{
	Q_ASSERT(data > 0 && data < 65535);
	int tempData = data / 26;
	if (tempData > 0) {
		int mode = data % 26;
		convert2ColName(mode, res);
		convert2ColName(tempData, res);
	}
	else {
		res = (to26AlphabetString(data) + res);
	}
}

QString LoadExcelData::to26AlphabetString(int data)
{
	QChar ch = data + 0x40;//A对应0x41
	return QString(ch);
}

void LoadExcelData::readExcelFast(QString fileName, QList<QList<QVariant>>& x_y)
{
	workbooks->querySubObject("Open(QString&)", fileName);//按文件路径打开已存在的工作簿
	workbook = excel->querySubObject("ActiveWorkBook");// 获取活动工作簿
	worksheets = workbook->querySubObject("WorkSheets");// 获取打开的excel文件中所有的工作sheet
	WorkSheetCount = worksheets->property("Count").toInt();//Excel文件中表的个数:
	worksheet = worksheets->querySubObject("Item(int)", 1);//获取第一个工作表，最后参数填1
	usedrange_Read = worksheet->querySubObject("UsedRange");//获取该sheet的数据范围（可以理解为有数据的矩形区域)
	//获取行数
	rows = usedrange_Read->querySubObject("Rows");
	RowsCount = rows->property("Count").toInt();
	//获取列数
	columns = usedrange_Read->querySubObject("Columns");
	ColumnsCount = columns->property("Count").toInt();
	//数据的起始行
	StartRow = rows->property("Row").toInt();
	//数据的起始列
	StartColumn = columns->property("Column").toInt();

	if (worksheet != NULL && !worksheet->isNull())
	{
		if (NULL == usedrange_Read || usedrange_Read->isNull())
		{
			return;
		}
		var = usedrange_Read->dynamicCall("Value");
		castVariant2ListListVariant(var, x_y); // 此函数将var转换成需要的格式
	}
	//要记得close，不然系统进程里会出现n个EXCEL.EXE进程
	workbook->dynamicCall("Close(bool)", false);  //关闭文件
	excel->dynamicCall("Quit()");
}

void LoadExcelData::writeExcelFast(const QList<QList<QVariant>>& x_y, QString fileName)
{
	if (x_y.isEmpty())
	{
		//保存的为空数据
		emit writeExcelisEmpty();
	}
	else
	{
		workbooks->dynamicCall("Add");//新建一个工作表。 新工作表将成为活动工作表
		workbook = excel->querySubObject("ActiveWorkBook");// 获取活动工作簿
		worksheet = workbook->querySubObject("Sheets(int)", 1); //获取第一个工作表，最后参数填1

		int row = x_y.size();//行数
		int col = x_y.at(0).size();//列数
		/*将列数转换成EXCEL中列的字母形式*/
		QString rangStr;
		convert2ColName(col, rangStr);
		rangStr += QString::number(row);
		rangStr = "A1:" + rangStr;
		usedrange_Write = worksheet->querySubObject("Range(const QString&)", rangStr);

		QVariant var;
		castListListVariant2Variant(x_y, var);
		usedrange_Write->setProperty("Value", var);

		workbook->dynamicCall("SaveCopyAs(QString)", QDir::toNativeSeparators(fileName));
		workbook->dynamicCall("Close(bool)", false);  //关闭文件
		excel->dynamicCall("Quit()");//关闭excel
	}
}

void LoadExcelData::setExcelCell(QAxObject * worksheet, int row, int column, QString text)
{
	QAxObject *cell = worksheet->querySubObject("Cells(int,int)", row, column);
	cell->setProperty("Value", text);
}