#include "LoadExcelDlg.h"

LoadExcelDlg::LoadExcelDlg(QWidget* parent /*= Q_NULLPTR*/)
	:QDialog(parent)
{
	ui.setupUi(this);
	ui.ExcelProgBar->setMinimum(0);  // 最小值
	ui.ExcelProgBar->setMaximum(0);  // 最大值
}

LoadExcelDlg::~LoadExcelDlg()
{
}