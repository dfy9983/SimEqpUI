#include "simeqpui.h"

SimEqpUI::SimEqpUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	qDebug() << "main thread:" << QThread::currentThreadId();
	//初始化ui
	initUi();

	/*********************** PMAC ***************************************************************************/
	myQPmac = new MyQPmac();

	/*********定时器****/
	plotTimer = new QTimer(this);
	plotTimer->setInterval(50);//设置定时器的间隔50ms
	plotTimer->setTimerType(Qt::PreciseTimer);//精确定时
	connect(plotTimer, &QTimer::timeout, this, &SimEqpUI::on_plotTimerTimeOut);

	getPmacDataTimer = new QTimer(this);
	getPmacDataTimer->setInterval(50);
	getPmacDataTimer->setTimerType(Qt::PreciseTimer);
	connect(getPmacDataTimer, &QTimer::timeout, this, &SimEqpUI::on_getPmacDataTimer);

	updateUiDataTimer = new QTimer(this);
	updateUiDataTimer->setInterval(100);
	updateUiDataTimer->setTimerType(Qt::CoarseTimer);
	connect(updateUiDataTimer, &QTimer::timeout, this, &SimEqpUI::on_updateUiDataTimer);
	updateUiDataTimer->start();

	/*********************EXCEL线程****************************************************************************/
	loadExcel_Qthread = new QThread();
	loadExcelThread = new ExcelThread();
	loadExcelThread->moveToThread(loadExcel_Qthread);
	//EXCEL线程开始
	loadExcel_Qthread->start();
	connect(loadExcel_Qthread, &QThread::started, loadExcelThread, &ExcelThread::startThread);
	connect(loadExcel_Qthread, &QThread::finished, loadExcelThread, &QObject::deleteLater);//终止线程时要调用deleteLater槽函数
	connect(loadExcel_Qthread, &QThread::finished, loadExcel_Qthread, &QObject::deleteLater);

	//读取标准0号曲线数据
	connect(this, &SimEqpUI::sig_ReadData, loadExcelThread, &ExcelThread::readData_Thread);
	//绘制Excel导入的 位移，速度或力 0号曲线
	connect(loadExcelThread, &ExcelThread::plotExcelData, this, &SimEqpUI::on_plotExcelData);
	//读取数据完成
	connect(loadExcelThread, &ExcelThread::readExcelFinish2main, this, &SimEqpUI::readExcelFinished);

	//写入接收1号曲线数据
	connect(this, &SimEqpUI::sig_WriteData, loadExcelThread, &ExcelThread::writeData_Thread);
	//写入数据为空警告
	connect(loadExcelThread, &ExcelThread::writeExcelIsEmpty2main, this, &SimEqpUI::on_warnWriteExcelIsEmpty);
	//写入数据完成
	connect(loadExcelThread, &ExcelThread::writeExcelFinish2main, this, &SimEqpUI::writeExcelFinished);

	/********************绘图********************************************************************************/
	//选中的曲线画笔
	QPen pen0(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	QPen pen1(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	selectedPen.append(pen0);
	selectedPen.append(pen1);
	//选中的图例
	YHfontBold = QFont("Microsoft YaHei", 10, QFont::Bold);
	//游标鼠标事件
	connect(widget_Disp, &QCustomPlot::mouseMove, curves_Disp, &CurvePlot::myMouseMoveEvent);
	connect(widget_Vel, &QCustomPlot::mouseMove, curves_Vel, &CurvePlot::myMouseMoveEvent);
	connect(widget_Force, &QCustomPlot::mouseMove, curves_Force, &CurvePlot::myMouseMoveEvent);

	//游标工具栏
	connect(ui.act_tracer, &QAction::toggled, curves_Disp, &CurvePlot::on_act_tracerToggled);
	connect(ui.act_tracer, &QAction::toggled, curves_Vel, &CurvePlot::on_act_tracerToggled);
	connect(ui.act_tracer, &QAction::toggled, curves_Force, &CurvePlot::on_act_tracerToggled);

	//框选放大工具栏
	connect(ui.act_zoomIn, &QAction::toggled, this, &SimEqpUI::on_actZoomInToggled);
	//拖动工具栏
	connect(ui.act_move, &QAction::toggled, this, &SimEqpUI::on_actMoveToggled);
	//选择曲线工具栏
	connect(ui.act_select, &QAction::toggled, this, &SimEqpUI::on_actSelectToggled);
	//右键菜单
	connect(widget_Disp, &QCustomPlot::customContextMenuRequested, this, &SimEqpUI::contextMenuRequestDisp);
	connect(widget_Vel, &QCustomPlot::customContextMenuRequested, this, &SimEqpUI::contextMenuRequestVel);
	connect(widget_Force, &QCustomPlot::customContextMenuRequested, this, &SimEqpUI::contextMenuRequestForce);

	//选中曲线图例或曲线
	connect(widget_Disp, &QCustomPlot::selectionChangedByUser, this, &SimEqpUI::selectionChanged_Disp);
	connect(widget_Vel, &QCustomPlot::selectionChangedByUser, this, &SimEqpUI::selectionChanged_Vel);
	connect(widget_Force, &QCustomPlot::selectionChangedByUser, this, &SimEqpUI::selectionChanged_Force);
}

SimEqpUI::~SimEqpUI()
{
	loadExcel_Qthread->quit();
	loadExcel_Qthread->wait();
	delete myQPmac;
}
void SimEqpUI::initUi()
{
	/*exceld读写对话框*/
	excelProssessDlg = new LoadExcelDlg(this);

	/*位移曲线窗口初始化*/
	widget_Disp = ui.widget_Disp;
	curves_Disp = new CurvePlot(widget_Disp);
	curves_Disp->setxAxisRange(0, 210);
	curves_Disp->setyAxisRange(-100, 150);
	curves_Disp->setxAxisName("时间 s");
	curves_Disp->setyAxisName("位移 mm");
	curves_Disp->setGraphName("目标曲线", "接收曲线");
	curves_Disp->setSelectLegend(false);//初始图例不可选

	/*速度曲线窗口初始化*/
	widget_Vel = ui.widget_Vel;

	curves_Vel = new CurvePlot(widget_Vel);
	curves_Vel->setxAxisRange(0, 210);
	curves_Vel->setyAxisRange(-350, 350);
	curves_Vel->setxAxisName("时间 s");
	curves_Vel->setyAxisName("速度 mm/s");
	curves_Vel->setGraphName("目标曲线", "接收曲线");
	curves_Vel->setSelectLegend(false);//初始图例不可选

	/*力曲线窗口初始化*/
	widget_Force = ui.widget_Force;
	curves_Force = new CurvePlot(widget_Force);
	curves_Force->setxAxisRange(0, 210);
	curves_Force->setyAxisRange(-2400, 2400);
	curves_Force->setxAxisName("时间 s");
	curves_Force->setyAxisName("力 Kg");
	curves_Force->setGraphName("目标曲线", "接收曲线");
	curves_Force->setSelectLegend(false);

	/*注册数据类型 跨线程传递*/
	qRegisterMetaType<QList<QList<QVariant>>>("QList<QList<QVariant>>");
	qRegisterMetaType<QList<QList<QVariant>>>("QList<QList<QVariant>>*");
	qRegisterMetaType<CurveType>("CurveType");
}

void SimEqpUI::addPoint(CurvePlot * graph, const double & x_temp, const double & y_temp)
{
	graph->x1.append(x_temp);
	graph->y1.append(y_temp);
	QList<QVariant> xy;
	xy.append(x_temp);
	xy.append(y_temp);
	graph->x_y1.append(xy);
}

void SimEqpUI::contextMenuRequestDisp(QPoint pos)
{
	QMenu *menu = new QMenu(this);
	menu->setStyleSheet("font:11pt;font-family:Microsoft YaHei");
	menu->setAttribute(Qt::WA_DeleteOnClose);
	menu->popup(widget_Disp->mapToGlobal(pos));
	menu->addAction("调整范围", curves_Disp, &CurvePlot::rescaleAxes);
	menu->addAction("清除选中曲线", curves_Disp, &CurvePlot::clearCurve);
}

void SimEqpUI::contextMenuRequestForce(QPoint pos)
{
	QMenu *menu = new QMenu(this);
	menu->setStyleSheet("font:11pt;font-family:Microsoft YaHei");
	menu->setAttribute(Qt::WA_DeleteOnClose);
	menu->popup(widget_Force->mapToGlobal(pos));
	menu->addAction("调整范围", curves_Force, &CurvePlot::rescaleAxes);
	menu->addAction("清除选中曲线", curves_Force, &CurvePlot::clearCurve);
}

void SimEqpUI::contextMenuRequestVel(QPoint pos)
{
	QMenu *menu = new QMenu(this);
	menu->setStyleSheet("font:11pt;font-family:Microsoft YaHei");
	menu->setAttribute(Qt::WA_DeleteOnClose);
	menu->popup(widget_Vel->mapToGlobal(pos));
	menu->addAction("调整范围", curves_Vel, &CurvePlot::rescaleAxes);
	menu->addAction("清除选中曲线", curves_Vel, &CurvePlot::clearCurve);
}

void SimEqpUI::on_MotorSevoOnButton_clicked()
{
	qDebug() << "MotorSevoOnButton clicked";
}

void SimEqpUI::on_ForceHomeButton_clicked()
{
	qDebug() << "ForceHomeButton clicked";
}

void SimEqpUI::on_StartButton_clicked()
{
	qDebug() << "StartButton clicked";
	switch (SimEqpGlobal::windSpeed)
	{
	case 10: 
		myQPmac->startWinds();
		break;
	case 15:

		break;
	case 20:

		break;

	}
	ui.StartButton->setText("正在运动...");
	ui.StopButton->setText("停止运动");
	ui.plotRealTimeCurve->setChecked(true);
	ui.ForceHomeButton->setEnabled(false);
	
}

void SimEqpUI::on_StopButton_clicked()
{
	qDebug() << "StopButton clicked";
	/*停止Pmac运动*/
	myQPmac->stopWinds();
	/*停止绘图*/
	ui.plotRealTimeCurve->setChecked(false);
	ui.StartButton->setText("开始运动");
	ui.StopButton->setText("已停止");
	ui.ForceHomeButton->setEnabled(true);
}

void SimEqpUI::on_RelDispJogButton_clicked()
{
	myQPmac->jogDisp(SimEqpGlobal::jog_disp);
}

void SimEqpUI::on_ConfirmParaButton_clicked()
{
	myQPmac->setJogVel(SimEqpGlobal::jog_vel);
	ui.RelDispJogButton->setEnabled(true);
	ui.LngPrsJogButton->setEnabled(true);
}

void SimEqpUI::on_JogVelEdit_textChanged()
{
	qDebug() << "JogVelEdit_textChanged";
	ui.RelDispJogButton->setEnabled(false);
	ui.LngPrsJogButton->setEnabled(false);
}

void SimEqpUI::on_JogDisEdit_textChanged()
{
	qDebug() << "JogDisEdit_textChanged";
	ui.RelDispJogButton->setEnabled(false);
	ui.LngPrsJogButton->setEnabled(false);
}

void SimEqpUI::on_plotRealTimeCurve_toggled(bool checked)
{
	/*开始绘图 仅仅绘图不运动*/
	if (checked)
	{
		qDebug() << "on_plotRealTimeCurve  is checked";
		if (!plotTimer->isActive())
		{
			plotTimer->start();
			SimEqpGlobal::startTime = QDateTime::currentDateTime();//获取开始时间
			qDebug() << "plotTimer->start()";
		}
		else
		{
			QMessageBox::information(NULL, "提示", "当前正在绘图！");
		}
	} 
	else
	{
		qDebug() << "on_plotRealTimeCurve  is  not checked";
		if (!plotTimer->isActive())
		{
			QMessageBox::information(NULL, "提示", "当前未在绘图！");
		}
		else
		{
			qDebug() << "plotTimer->stop()";
			plotTimer->stop();
		}
	}

}

void SimEqpUI::on_clearAllCurves_triggered()
{
	qDebug() << "clearAllCurves_triggered";
	ui.plotRealTimeCurve->setChecked(false);
	curves_Disp->clearAllCurves();
	curves_Vel->clearAllCurves();
	curves_Force->clearAllCurves();

}

void SimEqpUI::on_plotTimerTimeOut()
{
	//绘图时间
	SimEqpGlobal::endTime = QDateTime::currentDateTime();
	SimEqpGlobal::intervalTime = SimEqpGlobal::startTime.msecsTo(SimEqpGlobal::endTime) / 1000.0;

	//添加点
	addPoint(curves_Disp, SimEqpGlobal::intervalTime, SimEqpGlobal::rt_disp);
	addPoint(curves_Vel, SimEqpGlobal::intervalTime, SimEqpGlobal::rt_vel);
	addPoint(curves_Force, SimEqpGlobal::intervalTime, SimEqpGlobal::rt_force);
	//设置数据
	widget_Disp->graph(1)->setData(curves_Disp->x1, curves_Disp->y1);
	widget_Vel->graph(1)->setData(curves_Vel->x1, curves_Vel->y1);
	widget_Force->graph(1)->setData(curves_Force->x1, curves_Force->y1);
	//重新绘制
	widget_Disp->replot(QCustomPlot::rpQueuedReplot);
	widget_Vel->replot(QCustomPlot::rpQueuedReplot);
	widget_Force->replot(QCustomPlot::rpQueuedReplot);
}

void SimEqpUI::on_getPmacDataTimer()
{
	SimEqpGlobal::rt_disp = myQPmac->getMotorDisp();
	SimEqpGlobal::rt_vel = myQPmac->getMotorVel();
	SimEqpGlobal::rt_force = myQPmac->getMotorForce();
}

void SimEqpUI::on_updateUiDataTimer()
{
	QString strDisp = QString::number(SimEqpGlobal::rt_disp, 'f', 3);
	QString strVel = QString::number(SimEqpGlobal::rt_vel, 'f', 3);
	QString strForce = QString::number(SimEqpGlobal::rt_force, 'f', 3);
	
	ui.DispShowEdit->setText(strDisp);
	ui.VelShowEdit->setText(strVel);
	ui.ForceShowEdit->setText(strForce);

	SimEqpGlobal::jog_vel = ui.JogVelEdit->text().toDouble();
	SimEqpGlobal::jog_disp = ui.JogDisEdit->text().toDouble();
	
}

void SimEqpUI::on_LoadDataButton_clicked()
{
	qDebug() << "LoadDataButton clicked ";
	/*导入曲线选择菜单*/
	QMenu *selectLoadCurveMenu = new QMenu(this);
	selectLoadCurveMenu->setStyleSheet("font:11pt;font-family:Microsoft YaHei");//设置菜单字体
	selectLoadCurveMenu->addAction("导入风速曲线:10m/s", this, &SimEqpUI::on_LoadCurveWinds10);
	selectLoadCurveMenu->addAction("导入风速曲线:15m/s", this, &SimEqpUI::on_LoadCurveWinds15);
	selectLoadCurveMenu->addAction("导入风速曲线:20m/s", this, &SimEqpUI::on_LoadCurveWinds20);

	ui.LoadDataButton->setMenu(selectLoadCurveMenu);
	selectLoadCurveMenu->exec(QPoint(QCursor::pos().x(), QCursor::pos().y()));//解决需要点击两次问题
}

void SimEqpUI::on_LoadCurveWinds10()
{
	//qDebug() << "工作目录:" << QDir::currentPath();
	/*选择excel文件
	QString strFile = QFileDialog::getOpenFileName(this, "选择文件", "./", "文本文件(*.xls;*.xlsx;)");
	*/
	/*直接加载现有文件*/
	//QString strFile = "E:/0Study/Graduate 3/CppWorkSpace/SimulationEquipment/bin/10m_sWinds_Disp.xlsx";
	/*拼接 获取文件绝对路径--标准曲线*/
	QString strFile0 = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/ExperimentData";
	QString strFileDisp = strFile0 + "/10m_sWinds_Disp.xlsx";
	QString strFileVel = strFile0 + "/10m_sWinds_Vel.xlsx";
	QString strFileForce = strFile0 + "/10m_sWinds_Force.xlsx";
	qDebug() << "读取的文件路径:" << strFileDisp;

	on_LoadDispCurve(strFileDisp);//读取位移
	on_LoadVelCurve(strFileVel);//读取速度
	on_LoadForceCurce(strFileForce);//读取力

	ui.LoadDataButton->setText("选择曲线(10m/s风速)");
	SimEqpGlobal::windSpeed = 10;

	/*下载力运动程序至PMac*/
	QString strFile_PmacProg = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/PmacProg";
	QString strMoveFile = strFile_PmacProg + "/8_F_PLC_ms10_05_new.pmc";
	myQPmac->downloadFile(strMoveFile);
}

void SimEqpUI::on_LoadCurveWinds15()
{
	QString strFile0 = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/ExperimentData";
	QString strFileDisp = strFile0 + "/15m_sWinds_Disp.xlsx";
	QString strFileVel = strFile0 + "/15m_sWinds_Vel.xlsx";
	QString strFileForce = strFile0 + "/15m_sWinds_Force.xlsx";
	qDebug() << "读取的文件路径:" << strFileDisp;

	on_LoadDispCurve(strFileDisp);//读取位移
	on_LoadVelCurve(strFileVel);//读取速度
	on_LoadForceCurce(strFileForce);//读取力

	ui.LoadDataButton->setText("选择曲线(15m/s风速)");
	SimEqpGlobal::windSpeed = 15;

	///*下载力运动程序至PMac*/
	//QString strFile_PmacProg = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/PmacProg";
	//QString strMoveFile = strFile_PmacProg + "/8_F_PLC_ms10_05_new.pmc";
	//myQPmac->downloadFile(strMoveFile);

}

void SimEqpUI::on_LoadCurveWinds20()
{
	QString strFile0 = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/ExperimentData";
	QString strFileDisp = strFile0 + "/20m_sWinds_Disp.xlsx";
	QString strFileVel = strFile0 + "/20m_sWinds_Vel.xlsx";
	QString strFileForce = strFile0 + "/20m_sWinds_Force.xlsx";
	qDebug() << "读取的文件路径:" << strFileDisp;

	on_LoadDispCurve(strFileDisp);//读取位移
	on_LoadDispCurve(strFileVel);//读取速度
	on_LoadForceCurce(strFileForce);//读取力

	ui.LoadDataButton->setText("选择曲线(20m/s风速)");
	SimEqpGlobal::windSpeed = 20;

	///*下载力运动程序至PMac*/
	//QString strFile_PmacProg = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/PmacProg";
	//QString strMoveFile = strFile_PmacProg + "/8_F_PLC_ms10_05_new.pmc";
	//myQPmac->downloadFile(strMoveFile);

}
void SimEqpUI::on_LoadDispCurve(QString strFile)
{
	/*判断文件是否存在，存在则读取*/
	if (!strFile.isEmpty())
	{
		if (QFileInfo(strFile).exists() == false)
		{
			qDebug() << "文件不存在！";
			QMessageBox::information(NULL, "提示", "位移曲线 文件不存在！");
		}
		else
		{
			curves_Disp->x0.clear();
			curves_Disp->y0.clear();
			curves_Disp->x_y0.clear();
			qDebug() << "emit sig_ReadData Dis";
			emit sig_ReadData(Disp, strFile, &curves_Disp->x_y0);
			excelProssessDlg->exec();//显示正在读写对话框
		}
	}
	else
	{
		qDebug() << "位移曲线 文件名为空";
	}
}

void SimEqpUI::on_LoadVelCurve(QString strFile)
{
	/*判断文件是否存在，存在则读取*/
	if (!strFile.isEmpty())
	{
		if (QFileInfo(strFile).exists() == false)
		{
			qDebug() << "速度曲线 文件不存在！";
			QMessageBox::information(NULL, "提示", "速度曲线文件不存在！");
		}
		else
		{
			curves_Vel->x0.clear();
			curves_Vel->y0.clear();
			curves_Vel->x_y0.clear();
			qDebug() << "emit sig_ReadData Vel";
			emit sig_ReadData(Vel, strFile, &curves_Vel->x_y0);
			excelProssessDlg->exec();//显示正在读写对话框
		}
	}
	else
	{
		qDebug() << "速度曲线 文件名为空";
	}

}

void SimEqpUI::on_LoadForceCurce(QString strFile)
{
	/*判断文件是否存在，存在则读取*/
	if (!strFile.isEmpty())
	{
		if (QFileInfo(strFile).exists() == false)
		{
			qDebug() << "力曲线 文件不存在！";
			QMessageBox::information(NULL, "提示", "力曲线文件不存在！");
		}
		else
		{
			curves_Force->x0.clear();
			curves_Force->y0.clear();
			curves_Force->x_y0.clear();
			qDebug() << "emit sig_ReadData Force";
			emit sig_ReadData(Force, strFile, &curves_Force->x_y0);
			excelProssessDlg->exec();//显示正在读写对话框
		}
	}
	else
	{
		qDebug() << "力曲线 文件名为空";
	}
}

void SimEqpUI::on_RecordDataButton_clicked()
{
	qDebug() << "RecordDataButton clicked ";
	/*写入曲线选择菜单*/
	QMenu *selectRecordCurveMenu = new QMenu(this);
	selectRecordCurveMenu->setStyleSheet("font:11pt;font-family:Microsoft YaHei");//设置菜单字体
	selectRecordCurveMenu->addAction("记录接收位移曲线1", this, &SimEqpUI::on_RecordDispCurve);
	selectRecordCurveMenu->addAction("记录接收速度曲线1", this, &SimEqpUI::on_RecordVelCurce);
	selectRecordCurveMenu->addAction("记录接收力曲线1", this, &SimEqpUI::on_RecordForceCurce);

	ui.RecordDataButton->setMenu(selectRecordCurveMenu);
	selectRecordCurveMenu->exec(QPoint(QCursor::pos().x(), QCursor::pos().y()));//解决需要点击两次问题
}

void SimEqpUI::on_RecordDispCurve()
{
	QString strFile = QFileDialog::getSaveFileName(this, "另存为", "./", "文本文件(*.xlsx;)");
	if (!strFile.isEmpty())
	{
		emit sig_WriteData(Disp, &curves_Disp->x_y1, strFile);
		excelProssessDlg->exec();//显示正在读写对话框
	}
}

void SimEqpUI::on_RecordForceCurce()
{
	QString strFile = QFileDialog::getSaveFileName(this, "另存为", "./", "文本文件(*.xlsx;)");
	if (!strFile.isEmpty())
	{
		emit sig_WriteData(Force, &curves_Force->x_y1, strFile);
		excelProssessDlg->exec();//显示正在读写对话框
	}
}

void SimEqpUI::on_RecordVelCurce()
{
	QString strFile = QFileDialog::getSaveFileName(this, "另存为", "./", "文本文件(*.xlsx;)");
	if (!strFile.isEmpty())
	{
		emit sig_WriteData(Vel, &curves_Vel->x_y1, strFile);
		excelProssessDlg->exec();//显示正在读写对话框
	}
}

void SimEqpUI::writeExcelFinished()
{
	qDebug() << "ui:写入Excel完成";
	excelProssessDlg->close();
}

void SimEqpUI::on_warnWriteExcelIsEmpty()
{
	QMessageBox::warning(this, "警告", "保存曲线为空，无法保存！");
}

void SimEqpUI::readExcelFinished()
{
	qDebug() << "ui:读取Excel完成";
	excelProssessDlg->close();
}

void SimEqpUI::on_plotExcelData(CurveType curve)
{
	switch (curve)
	{
	case Disp:
		LoadExcelData::castListListVariant2VectorAndVector(curves_Disp->x_y0, curves_Disp->x0, curves_Disp->y0);
		widget_Disp->graph(0)->setData(curves_Disp->x0, curves_Disp->y0);
		widget_Disp->replot();
		qDebug() << "位移0曲线 绘制完成";
		break;
	case Vel:
		LoadExcelData::castListListVariant2VectorAndVector(curves_Vel->x_y0, curves_Vel->x0, curves_Vel->y0);
		widget_Vel->graph(0)->setData(curves_Vel->x0, curves_Vel->y0);
		widget_Vel->replot();
		qDebug() << "速度0曲线 绘制完成";
		break;
	case Force:
		LoadExcelData::castListListVariant2VectorAndVector(curves_Force->x_y0, curves_Force->x0, curves_Force->y0);
		widget_Force->graph(0)->setData(curves_Force->x0, curves_Force->y0);
		widget_Force->replot();
		qDebug() << "力0曲线 绘制完成";
		break;
	default:
		break;
	}
}

void SimEqpUI::on_actZoomInToggled(bool arg1)
{
	if (arg1)
	{
		qDebug() << "act_zoomIn is on!";
		ui.act_move->setChecked(false);//取消拖动选项
		widget_Disp->setInteraction(QCP::iRangeDrag, false);//取消拖动
		widget_Disp->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);
		widget_Force->setInteraction(QCP::iRangeDrag, false);//取消拖动
		widget_Force->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);
		widget_Vel->setInteraction(QCP::iRangeDrag, false);//取消拖动
		widget_Vel->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);
	}
	else
	{
		qDebug() << "act_zoomIn is off!";
		widget_Disp->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Vel->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Force->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
	}
}

void SimEqpUI::on_actMoveToggled(bool arg1)
{
	if (arg1)
	{
		qDebug() << "act_move is on!";
		ui.act_zoomIn->setChecked(false);//取消框选放大选项
		ui.act_select->setChecked(false);
		widget_Disp->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Disp->setInteraction(QCP::iRangeDrag, true);//使能拖动
		widget_Force->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Force->setInteraction(QCP::iRangeDrag, true);//使能拖动
		widget_Vel->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Vel->setInteraction(QCP::iRangeDrag, true);//使能拖动
	}
	else
	{
		qDebug() << "act_move is off!";
		widget_Disp->setInteraction(QCP::iRangeDrag, false);
		widget_Vel->setInteraction(QCP::iRangeDrag, false);
		widget_Force->setInteraction(QCP::iRangeDrag, false);
	}
}

void SimEqpUI::on_actSelectToggled(bool arg1)
{
	if (arg1)
	{
		qDebug() << "act_select is on!";

		ui.act_zoomIn->setChecked(false);//取消框选放大选项
		ui.act_move->setChecked(false);

		curves_Disp->setSelectLegend(true);
		widget_Disp->setInteraction(QCP::iSelectPlottables, true);//曲线可选
		widget_Disp->graph(0)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Disp->graph(1)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Disp->setSelectionRectMode(QCP::SelectionRectMode::srmSelect);

		curves_Vel->setSelectLegend(true);
		widget_Vel->setInteraction(QCP::iSelectPlottables, true);//曲线可选
		widget_Vel->graph(0)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Vel->graph(1)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Vel->setSelectionRectMode(QCP::SelectionRectMode::srmSelect);

		curves_Force->setSelectLegend(true);
		widget_Force->setInteraction(QCP::iSelectPlottables, true);//曲线可选
		widget_Force->graph(0)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Force->graph(1)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Force->setSelectionRectMode(QCP::SelectionRectMode::srmSelect);
	}
	else
	{
		qDebug() << "act_select is off!";
		curves_Disp->setSelectLegend(false);
		curves_Vel->setSelectLegend(false);
		curves_Force->setSelectLegend(false);

		widget_Disp->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Disp->setInteraction(QCP::iSelectPlottables, false);
		widget_Vel->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Vel->setInteraction(QCP::iSelectPlottables, false);
		widget_Force->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Force->setInteraction(QCP::iSelectPlottables, false);
	}
}

void SimEqpUI::selectionChanged_Disp()
{
	qDebug() << "selectionChanged_Disp";
	bool noneSelected = true;
	for (int i = 0; i < widget_Disp->graphCount(); ++i)
	{
		QCPGraph *graph = widget_Disp->graph(i);
		QCPPlottableLegendItem *item = widget_Disp->legend->itemWithPlottable(graph);
		if (item->selected() || graph->selected())
		{
			noneSelected = false;
			curves_Disp->curIndex = i;
			item->setSelected(true);
			//设置选中线样式
			graph->selectionDecorator()->setPen(selectedPen[i]);
			//设置选中图例字体和颜色
			item->setSelectedFont(YHfontBold);
			item->setSelectedTextColor(Qt::black);
			//设置选中部分数据
			graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
		}
		else item->setSelected(false);
	}
	if (noneSelected) curves_Disp->curIndex = -1;
	qDebug() << "curves_Disp->curIndex = " << curves_Disp->curIndex;
}

void SimEqpUI::selectionChanged_Force()
{
	qDebug() << "selectionChanged_Force";
	bool noneSelected = true;
	for (int i = 0; i < widget_Force->graphCount(); ++i)
	{
		QCPGraph *graph = widget_Force->graph(i);
		QCPPlottableLegendItem *item = widget_Force->legend->itemWithPlottable(graph);
		if (item->selected() || graph->selected())
		{
			noneSelected = false;
			curves_Force->curIndex = i;
			item->setSelected(true);
			graph->selectionDecorator()->setPen(selectedPen[i]);
			item->setSelectedFont(YHfontBold);
			item->setSelectedTextColor(Qt::black);

			graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
		}
		else item->setSelected(false);
	}
	if (noneSelected) curves_Force->curIndex = -1;
	qDebug() << "curves_Force->curIndex = " << curves_Force->curIndex;
}

void SimEqpUI::selectionChanged_Vel()
{
	qDebug() << "selectionChanged_Vel";
	bool noneSelected = true;
	for (int i = 0; i < widget_Vel->graphCount(); ++i)
	{
		QCPGraph *graph = widget_Vel->graph(i);
		QCPPlottableLegendItem *item = widget_Vel->legend->itemWithPlottable(graph);
		if (item->selected() || graph->selected())
		{
			noneSelected = false;
			curves_Vel->curIndex = i;
			item->setSelected(true);
			graph->selectionDecorator()->setPen(selectedPen[i]);
			item->setSelectedFont(YHfontBold);
			item->setSelectedTextColor(Qt::black);

			graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
		}
		else item->setSelected(false);
	}
	if (noneSelected) curves_Vel->curIndex = -1;
	qDebug() << "curves_Vel->curIndex = " << curves_Vel->curIndex;
}



void SimEqpUI::delay_block(int ms)
{
	QTime t = QTime::currentTime().addMSecs(ms);
	while (QTime::currentTime()<t);//单位ms
	{
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}
}

void SimEqpUI::delay_nonblock(int ms)
{
	QTime t = QTime::currentTime();
	QTime nowT;
	do 
	{
		nowT = QTime::currentTime();
	} 
	while (t.msecsTo(nowT) <= ms);
}

void SimEqpUI::on_SelectControlModeButton_clicked()
{
	qDebug() << "SelectControlModeButton is clicked";
	QMenu * selectControlModeMenu = new QMenu(this);
	selectControlModeMenu->addAction(ui.CurveMode_Force);
	selectControlModeMenu->addAction(ui.JogMode_Vel);
	ui.SelectControlModeButton->setMenu(selectControlModeMenu);
	selectControlModeMenu->exec(QPoint(QCursor::pos().x(), QCursor::pos().y()));//解决需要点击两次问题
}

void SimEqpUI::on_CurveMode_Force_triggered()
{
	QMessageBox::StandardButton answer = QMessageBox::information(NULL, "提示",
		"<font size='5'>请确认是否将控制柜的旋钮调至\
		<font size='6' color='red'><b><p align='center'>\
		转矩Mode</p></b>\
		</font>\
		</font>", 
		QMessageBox::Yes | QMessageBox::No, QMessageBox::NoButton);
	if (answer == QMessageBox::Yes)
	{
		ui.CurveControlBox->setEnabled(true);
		ui.JogControlBox->setEnabled(false);
		ui.SelectControlModeButton->setText("选择控制模式(力控制)");
		myQPmac->openForceLoop();//力开环

	}
}

void SimEqpUI::on_JogMode_Vel_triggered()
{
	QMessageBox::StandardButton answer = QMessageBox::information(NULL, "提示",
		"<font size='5'>请确认是否将控制柜的旋钮调至\
		<font size='6' color='red'><b><p align='center'>\
		速度Mode</p></b>\
		</font>\
		</font>",
		QMessageBox::Yes | QMessageBox::No, QMessageBox::NoButton);
	if (answer == QMessageBox::Yes)
	{
		ui.CurveControlBox->setEnabled(false);
		ui.JogControlBox->setEnabled(true);
		ui.SelectControlModeButton->setText("选择控制模式(速度控制)");

	}

}

void SimEqpUI::on_ConnectPMACButton_clicked()
{
	qDebug() << "QPmac select clicked";
	bool isConnected;
	isConnected = myQPmac->creatPmacSelect();
	if (isConnected)
	{
		ui.InitPmacButton->setEnabled(true);
	} 

}

void SimEqpUI::on_InitPmacButton_clicked()
{
	qDebug() << "InitPmacButton is clicked";

	//初始化
	bool initState  = myQPmac->initPmac();
	/*******延时1s*****/
	delay_nonblock(1000);
	/***************/
	//开启力滤波
	if (initState)
	{
		myQPmac->enablePLC(1);
	}

	getPmacDataTimer->start();
	ui.SelectControlModeButton->setEnabled(true);
	ui.MotorSevoOnButton->setEnabled(true);

	ui.InitPmacButton->setText("初始化完成");

}
