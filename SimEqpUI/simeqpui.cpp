#include "simeqpui.h"

SimEqpUI::SimEqpUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	qDebug() << "main thread:" << QThread::currentThreadId();
	//��ʼ��ui
	initUi();

	/*********************** PMAC ***************************************************************************/
	myQPmac = new MyQPmac();

	/*********��ʱ��****/
	plotTimer = new QTimer(this);
	plotTimer->setInterval(50);//���ö�ʱ���ļ��50ms
	plotTimer->setTimerType(Qt::PreciseTimer);//��ȷ��ʱ
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

	/*********************EXCEL�߳�****************************************************************************/
	loadExcel_Qthread = new QThread();
	loadExcelThread = new ExcelThread();
	loadExcelThread->moveToThread(loadExcel_Qthread);
	//EXCEL�߳̿�ʼ
	loadExcel_Qthread->start();
	connect(loadExcel_Qthread, &QThread::started, loadExcelThread, &ExcelThread::startThread);
	connect(loadExcel_Qthread, &QThread::finished, loadExcelThread, &QObject::deleteLater);//��ֹ�߳�ʱҪ����deleteLater�ۺ���
	connect(loadExcel_Qthread, &QThread::finished, loadExcel_Qthread, &QObject::deleteLater);

	//��ȡ��׼0����������
	connect(this, &SimEqpUI::sig_ReadData, loadExcelThread, &ExcelThread::readData_Thread);
	//����Excel����� λ�ƣ��ٶȻ��� 0������
	connect(loadExcelThread, &ExcelThread::plotExcelData, this, &SimEqpUI::on_plotExcelData);
	//��ȡ�������
	connect(loadExcelThread, &ExcelThread::readExcelFinish2main, this, &SimEqpUI::readExcelFinished);

	//д�����1����������
	connect(this, &SimEqpUI::sig_WriteData, loadExcelThread, &ExcelThread::writeData_Thread);
	//д������Ϊ�վ���
	connect(loadExcelThread, &ExcelThread::writeExcelIsEmpty2main, this, &SimEqpUI::on_warnWriteExcelIsEmpty);
	//д���������
	connect(loadExcelThread, &ExcelThread::writeExcelFinish2main, this, &SimEqpUI::writeExcelFinished);

	/********************��ͼ********************************************************************************/
	//ѡ�е����߻���
	QPen pen0(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	QPen pen1(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
	selectedPen.append(pen0);
	selectedPen.append(pen1);
	//ѡ�е�ͼ��
	YHfontBold = QFont("Microsoft YaHei", 10, QFont::Bold);
	//�α�����¼�
	connect(widget_Disp, &QCustomPlot::mouseMove, curves_Disp, &CurvePlot::myMouseMoveEvent);
	connect(widget_Vel, &QCustomPlot::mouseMove, curves_Vel, &CurvePlot::myMouseMoveEvent);
	connect(widget_Force, &QCustomPlot::mouseMove, curves_Force, &CurvePlot::myMouseMoveEvent);

	//�α깤����
	connect(ui.act_tracer, &QAction::toggled, curves_Disp, &CurvePlot::on_act_tracerToggled);
	connect(ui.act_tracer, &QAction::toggled, curves_Vel, &CurvePlot::on_act_tracerToggled);
	connect(ui.act_tracer, &QAction::toggled, curves_Force, &CurvePlot::on_act_tracerToggled);

	//��ѡ�Ŵ󹤾���
	connect(ui.act_zoomIn, &QAction::toggled, this, &SimEqpUI::on_actZoomInToggled);
	//�϶�������
	connect(ui.act_move, &QAction::toggled, this, &SimEqpUI::on_actMoveToggled);
	//ѡ�����߹�����
	connect(ui.act_select, &QAction::toggled, this, &SimEqpUI::on_actSelectToggled);
	//�Ҽ��˵�
	connect(widget_Disp, &QCustomPlot::customContextMenuRequested, this, &SimEqpUI::contextMenuRequestDisp);
	connect(widget_Vel, &QCustomPlot::customContextMenuRequested, this, &SimEqpUI::contextMenuRequestVel);
	connect(widget_Force, &QCustomPlot::customContextMenuRequested, this, &SimEqpUI::contextMenuRequestForce);

	//ѡ������ͼ��������
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
	/*exceld��д�Ի���*/
	excelProssessDlg = new LoadExcelDlg(this);
	//����lineedit��������
	QRegExp velRange("^-?(30|[1-2]?\\d(\\.\\d{1,2})?)$");//-30~+30 ��λС��
	QRegExpValidator *pRegVel = new QRegExpValidator(velRange, this);
	QRegExp dispRange("^-?(250|[1-2]?[0-4]?\\d(\\.\\d{1,2})?)$");//-250~+250 ��λС��
	QRegExpValidator *pRegDisp = new QRegExpValidator(dispRange, this);
	ui.JogVelEdit->setValidator(pRegVel);
	ui.JogDisEdit->setValidator(pRegDisp);

	/*λ�����ߴ��ڳ�ʼ��*/
	widget_Disp = ui.widget_Disp;
	curves_Disp = new CurvePlot(widget_Disp);
	curves_Disp->setxAxisRange(0, 210);
	curves_Disp->setyAxisRange(-100, 150);
	curves_Disp->setxAxisName("ʱ�� s");
	curves_Disp->setyAxisName("λ�� mm");
	curves_Disp->setGraphName("Ŀ������", "��������");
	curves_Disp->setSelectLegend(false);//��ʼͼ������ѡ

	/*�ٶ����ߴ��ڳ�ʼ��*/
	widget_Vel = ui.widget_Vel;

	curves_Vel = new CurvePlot(widget_Vel);
	curves_Vel->setxAxisRange(0, 210);
	curves_Vel->setyAxisRange(-350, 350);
	curves_Vel->setxAxisName("ʱ�� s");
	curves_Vel->setyAxisName("�ٶ� mm/s");
	curves_Vel->setGraphName("Ŀ������", "��������");
	curves_Vel->setSelectLegend(false);//��ʼͼ������ѡ

	/*�����ߴ��ڳ�ʼ��*/
	widget_Force = ui.widget_Force;
	curves_Force = new CurvePlot(widget_Force);
	curves_Force->setxAxisRange(0, 210);
	curves_Force->setyAxisRange(-2400, 2400);
	curves_Force->setxAxisName("ʱ�� s");
	curves_Force->setyAxisName("�� Kg");
	curves_Force->setGraphName("Ŀ������", "��������");
	curves_Force->setSelectLegend(false);

	/*ע���������� ���̴߳���*/
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
	menu->addAction("������Χ", curves_Disp, &CurvePlot::rescaleAxes);
	menu->addAction("���ѡ������", curves_Disp, &CurvePlot::clearCurve);
}

void SimEqpUI::contextMenuRequestForce(QPoint pos)
{
	QMenu *menu = new QMenu(this);
	menu->setStyleSheet("font:11pt;font-family:Microsoft YaHei");
	menu->setAttribute(Qt::WA_DeleteOnClose);
	menu->popup(widget_Force->mapToGlobal(pos));
	menu->addAction("������Χ", curves_Force, &CurvePlot::rescaleAxes);
	menu->addAction("���ѡ������", curves_Force, &CurvePlot::clearCurve);
}

void SimEqpUI::contextMenuRequestVel(QPoint pos)
{
	QMenu *menu = new QMenu(this);
	menu->setStyleSheet("font:11pt;font-family:Microsoft YaHei");
	menu->setAttribute(Qt::WA_DeleteOnClose);
	menu->popup(widget_Vel->mapToGlobal(pos));
	menu->addAction("������Χ", curves_Vel, &CurvePlot::rescaleAxes);
	menu->addAction("���ѡ������", curves_Vel, &CurvePlot::clearCurve);
}

//void SimEqpUI::on_MotorSevoOnButton_clicked()
//{
//	qDebug() << "MotorSevoOnButton clicked";
//}

void SimEqpUI::on_ForceHomeButton_clicked()
{
	qDebug() << "ForceHomeButton clicked";
	myQPmac->forceHome();
	ui.ForceHomeButton->setText("����������...");
	ui.StopButton->setText("ֹͣ�˶�");

}

void SimEqpUI::on_StartButton_clicked()
{
	qDebug() << "StartButton clicked";
	//��ʼ�˶�
	myQPmac->startWinds();

	ui.StartButton->setText("�����˶�...");
	ui.StopButton->setText("ֹͣ�˶�");
	ui.plotRealTimeCurve->setChecked(true);
	ui.ForceHomeButton->setEnabled(false);
	
}

void SimEqpUI::on_StopButton_clicked()
{
	qDebug() << "StopButton is clicked";
	/*ֹͣPmac�˶�*/
	myQPmac->stopMove();
	/*ֹͣ��ͼ*/
	ui.plotRealTimeCurve->setChecked(false);

	ui.StartButton->setText("��ʼ�˶�");
	ui.ForceHomeButton->setText("������");
	ui.StopButton->setText("��ֹͣ");
	ui.ForceHomeButton->setEnabled(true);
}

void SimEqpUI::on_RelDispJogButton_clicked()
{
	qDebug() << "RelDispJogButton is clicked";
	myQPmac->jogDisp(SimEqpGlobal::jog_disp);
}

void SimEqpUI::on_LngPrsJogPosButton_pressed()
{
	qDebug() << "Jog+ is pressed";
	myQPmac->jogPosContinuously();
}

void SimEqpUI::on_LngPrsJogPosButton_released()
{
	qDebug() << "jog+ is released";
	myQPmac->closeLoop();
}

void SimEqpUI::on_LngPrsJogNegButton_pressed()
{
	qDebug() << "Jog- is pressed";
	myQPmac->jogNegContinuously();
}

void SimEqpUI::on_LngPrsJogNegButton_released()
{
	qDebug() << "jog- is released";
	myQPmac->closeLoop();
}

void SimEqpUI::on_ConfirmParaButton_clicked()
{
	myQPmac->setJogVel(SimEqpGlobal::jog_vel);
	ui.RelDispJogButton->setEnabled(true);
	ui.LngPrsJogPosButton->setEnabled(true);
	ui.LngPrsJogNegButton->setEnabled(true);
}

void SimEqpUI::on_JogVelEdit_textChanged()
{
	qDebug() << "JogVelEdit_textChanged";
	ui.RelDispJogButton->setEnabled(false);
	ui.LngPrsJogPosButton->setEnabled(false);
	ui.LngPrsJogNegButton->setEnabled(false);
}

void SimEqpUI::on_JogDisEdit_textChanged()
{
	qDebug() << "JogDisEdit_textChanged";
	ui.RelDispJogButton->setEnabled(false);
	ui.LngPrsJogPosButton->setEnabled(false);
	ui.LngPrsJogNegButton->setEnabled(false);
}

void SimEqpUI::on_plotRealTimeCurve_toggled(bool checked)
{
	/*��ʼ��ͼ ������ͼ���˶�*/
	if (checked)
	{
		qDebug() << "on_plotRealTimeCurve  is checked";
		if (!plotTimer->isActive())
		{
			plotTimer->start();
			SimEqpGlobal::startTime = QDateTime::currentDateTime();//��ȡ��ʼʱ��
			qDebug() << "plotTimer->start()";
		}
		else
		{
			QMessageBox::information(NULL, "��ʾ", "��ǰ���ڻ�ͼ��");
		}
	} 
	else
	{
		qDebug() << "on_plotRealTimeCurve  is  not checked";
		if (!plotTimer->isActive())
		{
			QMessageBox::information(NULL, "��ʾ", "��ǰδ�ڻ�ͼ��");
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
	//��ͼʱ��
	SimEqpGlobal::endTime = QDateTime::currentDateTime();
	SimEqpGlobal::intervalTime = SimEqpGlobal::startTime.msecsTo(SimEqpGlobal::endTime) / 1000.0;

	//��ӵ�
	addPoint(curves_Disp, SimEqpGlobal::intervalTime, SimEqpGlobal::rt_disp);
	addPoint(curves_Vel, SimEqpGlobal::intervalTime, SimEqpGlobal::rt_vel);
	addPoint(curves_Force, SimEqpGlobal::intervalTime, SimEqpGlobal::rt_force);
	//��������
	widget_Disp->graph(1)->setData(curves_Disp->x1, curves_Disp->y1);
	widget_Vel->graph(1)->setData(curves_Vel->x1, curves_Vel->y1);
	widget_Force->graph(1)->setData(curves_Force->x1, curves_Force->y1);
	//���»���
	widget_Disp->replot(QCustomPlot::rpQueuedReplot);
	widget_Vel->replot(QCustomPlot::rpQueuedReplot);
	widget_Force->replot(QCustomPlot::rpQueuedReplot);
}

void SimEqpUI::on_getPmacDataTimer()
{
	SimEqpGlobal::rt_disp = myQPmac->getMotorDisp();
	SimEqpGlobal::rt_vel = myQPmac->getMotorVel();
	SimEqpGlobal::rt_force = myQPmac->getMotorForce();
	SimEqpGlobal::posLimState = myQPmac->getPosLimState();
	SimEqpGlobal::negLimState = myQPmac->getNegLimState();
	SimEqpGlobal::forceHomeState = myQPmac->getForceHomeState();
	SimEqpGlobal::isMoving = myQPmac->getMoveState();
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

	if (!SimEqpGlobal::posLimState)
	{
		ui.PosLimitState->setStyleSheet("QLabel{background-color:rgb(229, 255, 238);}");
		ui.PosLimitState->setText("����λ-δ����");
	}
	else
	{
		ui.PosLimitState->setStyleSheet("QLabel{background-color:rgb(255, 161, 161);}");
		ui.PosLimitState->setText("����λ-�Ѵ���");
	}
	if (!SimEqpGlobal::negLimState)
	{
		ui.NegLimitState->setStyleSheet("QLabel{background-color:rgb(229, 255, 238);}");
		ui.NegLimitState->setText("����λ-δ����");
	}
	else
	{
		ui.NegLimitState->setStyleSheet("QLabel{background-color:rgb(255, 161, 161);}");
		ui.NegLimitState->setText("����λ-�Ѵ���");
	}

	if (!SimEqpGlobal::forceHomeState)
	{
		ui.StartButton->setEnabled(false);
		ui.ForceHomeButton->setText("������");
	}
	else
	{
		ui.StartButton->setEnabled(true);
		ui.ForceHomeButton->setText("�������");

	}

	if (!SimEqpGlobal::isMoving)
	{
		ui.MoveStateLabel->setStyleSheet("QLabel{background-color:rgb(229, 255, 238);}");
		if (ui.MoveStateLabel->text() == "�˶�״̬-�����˶�...")
		{
			ui.StopButton->setText("��ֹͣ");
			ui.StartButton->setText("��ʼ�˶�");
			ui.plotRealTimeCurve->setChecked(false);//ֹͣ��ͼ
		}
		ui.MoveStateLabel->setText("�˶�״̬-��ֹͣ");
	}
	else
	{
		ui.MoveStateLabel->setStyleSheet("QLabel{background-color:rgb(255, 225, 148);}");
		if (ui.MoveStateLabel->text() == "�˶�״̬-��ֹͣ")
		{
			ui.StartButton->setText("�����˶�...");
			ui.StopButton->setText("ֹͣ");
			
		}
		ui.MoveStateLabel->setText("�˶�״̬-�����˶�...");
	}
	
}

void SimEqpUI::on_LoadDataButton_clicked()
{
	qDebug() << "LoadDataButton clicked ";
	/*��������ѡ��˵�*/
	QMenu *selectLoadCurveMenu = new QMenu(this);
	selectLoadCurveMenu->setStyleSheet("font:11pt;font-family:Microsoft YaHei");//���ò˵�����
	selectLoadCurveMenu->addAction("�����������:10m/s", this, &SimEqpUI::on_LoadCurveWinds10);
	selectLoadCurveMenu->addAction("�����������:15m/s", this, &SimEqpUI::on_LoadCurveWinds15);
	selectLoadCurveMenu->addAction("�����������:20m/s", this, &SimEqpUI::on_LoadCurveWinds20);

	ui.LoadDataButton->setMenu(selectLoadCurveMenu);
	selectLoadCurveMenu->exec(QPoint(QCursor::pos().x(), QCursor::pos().y()));//�����Ҫ�����������
}

void SimEqpUI::on_LoadCurveWinds10()
{
	//qDebug() << "����Ŀ¼:" << QDir::currentPath();
	/*ѡ��excel�ļ�
	QString strFile = QFileDialog::getOpenFileName(this, "ѡ���ļ�", "./", "�ı��ļ�(*.xls;*.xlsx;)");
	*/
	/*ֱ�Ӽ��������ļ�*/
	//QString strFile = "E:/0Study/Graduate 3/CppWorkSpace/SimulationEquipment/bin/10m_sWinds_Disp.xlsx";
	/*ƴ�� ��ȡ�ļ�����·��--��׼����*/
	QString strFile0 = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/ExperimentData";
	QString strFileDisp = strFile0 + "/10m_sWinds_Disp.xlsx";
	QString strFileVel = strFile0 + "/10m_sWinds_Vel.xlsx";
	QString strFileForce = strFile0 + "/10m_sWinds_Force.xlsx";
	qDebug() << "��ȡ���ļ�·��:" << strFileDisp;

	on_LoadDispCurve(strFileDisp);//��ȡλ��
	on_LoadVelCurve(strFileVel);//��ȡ�ٶ�
	on_LoadForceCurce(strFileForce);//��ȡ��

	ui.LoadDataButton->setText("ѡ������(10m/s����)");
	SimEqpGlobal::windSpeed = 10;

	/*�������˶�������PMac*/
	QString strFile_PmacProg = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/PmacProg";
	QString strMoveFile = strFile_PmacProg + "/8_F_PLC_ms10_05.pmc";
	myQPmac->downloadFile(strMoveFile);
}

void SimEqpUI::on_LoadCurveWinds15()
{
	QString strFile0 = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/ExperimentData";
	QString strFileDisp = strFile0 + "/15m_sWinds_Disp.xlsx";
	QString strFileVel = strFile0 + "/15m_sWinds_Vel.xlsx";
	QString strFileForce = strFile0 + "/15m_sWinds_Force.xlsx";
	qDebug() << "��ȡ���ļ�·��:" << strFileDisp;

	on_LoadDispCurve(strFileDisp);//��ȡλ��
	on_LoadVelCurve(strFileVel);//��ȡ�ٶ�
	on_LoadForceCurce(strFileForce);//��ȡ��

	ui.LoadDataButton->setText("ѡ������(15m/s����)");
	SimEqpGlobal::windSpeed = 15;

	/*�������˶�������PMac*/
	QString strFile_PmacProg = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/PmacProg";
	QString strMoveFile = strFile_PmacProg + "/8_F_PLC_ms15_05.pmc";
	myQPmac->downloadFile(strMoveFile);

}

void SimEqpUI::on_LoadCurveWinds20()
{
	QString strFile0 = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/ExperimentData";
	QString strFileDisp = strFile0 + "/20m_sWinds_Disp.xlsx";
	QString strFileVel = strFile0 + "/20m_sWinds_Vel.xlsx";
	QString strFileForce = strFile0 + "/20m_sWinds_Force.xlsx";
	qDebug() << "��ȡ���ļ�·��:" << strFileDisp;

	on_LoadDispCurve(strFileDisp);//��ȡλ��
	on_LoadVelCurve(strFileVel);//��ȡ�ٶ�
	on_LoadForceCurce(strFileForce);//��ȡ��

	ui.LoadDataButton->setText("ѡ������(20m/s����)");
	SimEqpGlobal::windSpeed = 20;

	/*�������˶�������PMac*/
	QString strFile_PmacProg = QDir(QFileInfo(QDir::currentPath()).canonicalPath()).absoluteFilePath("doc") + "/PmacProg";
	QString strMoveFile = strFile_PmacProg + "/8_F_PLC_ms20_05.pmc";
	myQPmac->downloadFile(strMoveFile);

}
void SimEqpUI::on_LoadDispCurve(QString strFile)
{
	/*�ж��ļ��Ƿ���ڣ��������ȡ*/
	if (!strFile.isEmpty())
	{
		if (QFileInfo(strFile).exists() == false)
		{
			qDebug() << "�ļ������ڣ�";
			QMessageBox::information(NULL, "��ʾ", "λ������ �ļ������ڣ�");
		}
		else
		{
			curves_Disp->x0.clear();
			curves_Disp->y0.clear();
			curves_Disp->x_y0.clear();
			qDebug() << "emit sig_ReadData Dis";
			emit sig_ReadData(Disp, strFile, &curves_Disp->x_y0);
			excelProssessDlg->exec();//��ʾ���ڶ�д�Ի���
		}
	}
	else
	{
		qDebug() << "λ������ �ļ���Ϊ��";
	}
}

void SimEqpUI::on_LoadVelCurve(QString strFile)
{
	/*�ж��ļ��Ƿ���ڣ��������ȡ*/
	if (!strFile.isEmpty())
	{
		if (QFileInfo(strFile).exists() == false)
		{
			qDebug() << "�ٶ����� �ļ������ڣ�";
			QMessageBox::information(NULL, "��ʾ", "�ٶ������ļ������ڣ�");
		}
		else
		{
			curves_Vel->x0.clear();
			curves_Vel->y0.clear();
			curves_Vel->x_y0.clear();
			qDebug() << "emit sig_ReadData Vel";
			emit sig_ReadData(Vel, strFile, &curves_Vel->x_y0);
			excelProssessDlg->exec();//��ʾ���ڶ�д�Ի���
		}
	}
	else
	{
		qDebug() << "�ٶ����� �ļ���Ϊ��";
	}

}

void SimEqpUI::on_LoadForceCurce(QString strFile)
{
	/*�ж��ļ��Ƿ���ڣ��������ȡ*/
	if (!strFile.isEmpty())
	{
		if (QFileInfo(strFile).exists() == false)
		{
			qDebug() << "������ �ļ������ڣ�";
			QMessageBox::information(NULL, "��ʾ", "�������ļ������ڣ�");
		}
		else
		{
			curves_Force->x0.clear();
			curves_Force->y0.clear();
			curves_Force->x_y0.clear();
			qDebug() << "emit sig_ReadData Force";
			emit sig_ReadData(Force, strFile, &curves_Force->x_y0);
			excelProssessDlg->exec();//��ʾ���ڶ�д�Ի���
		}
	}
	else
	{
		qDebug() << "������ �ļ���Ϊ��";
	}
}

void SimEqpUI::on_RecordDataButton_clicked()
{
	qDebug() << "RecordDataButton clicked ";
	/*д������ѡ��˵�*/
	QMenu *selectRecordCurveMenu = new QMenu(this);
	selectRecordCurveMenu->setStyleSheet("font:11pt;font-family:Microsoft YaHei");//���ò˵�����
	selectRecordCurveMenu->addAction("��¼����λ������", this, &SimEqpUI::on_RecordDispCurve);
	selectRecordCurveMenu->addAction("��¼�����ٶ�����", this, &SimEqpUI::on_RecordVelCurce);
	selectRecordCurveMenu->addAction("��¼����������", this, &SimEqpUI::on_RecordForceCurce);

	ui.RecordDataButton->setMenu(selectRecordCurveMenu);
	selectRecordCurveMenu->exec(QPoint(QCursor::pos().x(), QCursor::pos().y()));//�����Ҫ�����������
}

void SimEqpUI::on_RecordDispCurve()
{
	QString strFile = QFileDialog::getSaveFileName(this, "���Ϊ", "./", "�ı��ļ�(*.xlsx;)");
	if (!strFile.isEmpty())
	{
		emit sig_WriteData(Disp, &curves_Disp->x_y1, strFile);
		excelProssessDlg->exec();//��ʾ���ڶ�д�Ի���
	}
}

void SimEqpUI::on_RecordForceCurce()
{
	QString strFile = QFileDialog::getSaveFileName(this, "���Ϊ", "./", "�ı��ļ�(*.xlsx;)");
	if (!strFile.isEmpty())
	{
		emit sig_WriteData(Force, &curves_Force->x_y1, strFile);
		excelProssessDlg->exec();//��ʾ���ڶ�д�Ի���
	}
}

void SimEqpUI::on_RecordVelCurce()
{
	QString strFile = QFileDialog::getSaveFileName(this, "���Ϊ", "./", "�ı��ļ�(*.xlsx;)");
	if (!strFile.isEmpty())
	{
		emit sig_WriteData(Vel, &curves_Vel->x_y1, strFile);
		excelProssessDlg->exec();//��ʾ���ڶ�д�Ի���
	}
}

void SimEqpUI::writeExcelFinished()
{
	qDebug() << "ui:д��Excel���";
	excelProssessDlg->close();
}

void SimEqpUI::on_warnWriteExcelIsEmpty()
{
	QMessageBox::warning(this, "����", "��������Ϊ�գ��޷����棡");
}

void SimEqpUI::readExcelFinished()
{
	qDebug() << "ui:��ȡExcel���";
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
		qDebug() << "λ��0���� �������";
		break;
	case Vel:
		LoadExcelData::castListListVariant2VectorAndVector(curves_Vel->x_y0, curves_Vel->x0, curves_Vel->y0);
		widget_Vel->graph(0)->setData(curves_Vel->x0, curves_Vel->y0);
		widget_Vel->replot();
		qDebug() << "�ٶ�0���� �������";
		break;
	case Force:
		LoadExcelData::castListListVariant2VectorAndVector(curves_Force->x_y0, curves_Force->x0, curves_Force->y0);
		widget_Force->graph(0)->setData(curves_Force->x0, curves_Force->y0);
		widget_Force->replot();
		qDebug() << "��0���� �������";
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
		ui.act_move->setChecked(false);//ȡ���϶�ѡ��
		widget_Disp->setInteraction(QCP::iRangeDrag, false);//ȡ���϶�
		widget_Disp->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);
		widget_Force->setInteraction(QCP::iRangeDrag, false);//ȡ���϶�
		widget_Force->setSelectionRectMode(QCP::SelectionRectMode::srmZoom);
		widget_Vel->setInteraction(QCP::iRangeDrag, false);//ȡ���϶�
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
		ui.act_zoomIn->setChecked(false);//ȡ����ѡ�Ŵ�ѡ��
		ui.act_select->setChecked(false);
		widget_Disp->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Disp->setInteraction(QCP::iRangeDrag, true);//ʹ���϶�
		widget_Force->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Force->setInteraction(QCP::iRangeDrag, true);//ʹ���϶�
		widget_Vel->setSelectionRectMode(QCP::SelectionRectMode::srmNone);
		widget_Vel->setInteraction(QCP::iRangeDrag, true);//ʹ���϶�
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

		ui.act_zoomIn->setChecked(false);//ȡ����ѡ�Ŵ�ѡ��
		ui.act_move->setChecked(false);

		curves_Disp->setSelectLegend(true);
		widget_Disp->setInteraction(QCP::iSelectPlottables, true);//���߿�ѡ
		widget_Disp->graph(0)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Disp->graph(1)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Disp->setSelectionRectMode(QCP::SelectionRectMode::srmSelect);

		curves_Vel->setSelectLegend(true);
		widget_Vel->setInteraction(QCP::iSelectPlottables, true);//���߿�ѡ
		widget_Vel->graph(0)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Vel->graph(1)->setSelectable(QCP::SelectionType::stMultipleDataRanges);
		widget_Vel->setSelectionRectMode(QCP::SelectionRectMode::srmSelect);

		curves_Force->setSelectLegend(true);
		widget_Force->setInteraction(QCP::iSelectPlottables, true);//���߿�ѡ
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
			//����ѡ������ʽ
			graph->selectionDecorator()->setPen(selectedPen[i]);
			//����ѡ��ͼ���������ɫ
			item->setSelectedFont(YHfontBold);
			item->setSelectedTextColor(Qt::black);
			//����ѡ�в�������
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
	while (QTime::currentTime()<t);//��λms
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
	selectControlModeMenu->exec(QPoint(QCursor::pos().x(), QCursor::pos().y()));//�����Ҫ�����������
}

void SimEqpUI::on_CurveMode_Force_triggered()
{
	//�л�����ģʽʱ�� �����������־λ
	myQPmac->setforceHomeState(false);

	QMessageBox::StandardButton answer = QMessageBox::information(NULL, "��ʾ",
		"<font size='5'>��ȷ���Ƿ񽫿��ƹ����ť����\
		<font size='6' color='red'><b><p align='center'>\
		ת��Mode</p></b>\
		</font>\
		</font>", 
		QMessageBox::Yes | QMessageBox::No, QMessageBox::NoButton);
	if (answer == QMessageBox::Yes)
	{
		ui.CurveControlBox->setEnabled(true);
		ui.JogControlBox->setEnabled(false);
		ui.SelectControlModeButton->setText("ѡ�����ģʽ(������)");
		myQPmac->openForceLoop();//������
	}
}

void SimEqpUI::on_JogMode_Vel_triggered()
{
	//�л�����ģʽʱ�� �����������־λ
	myQPmac->setforceHomeState(false);

	QMessageBox::StandardButton answer = QMessageBox::information(NULL, "��ʾ",
		"<font size='5'>��ȷ���Ƿ񽫿��ƹ����ť����\
		<font size='6' color='red'><b><p align='center'>\
		�ٶ�Mode</p></b>\
		</font>\
		</font>",
		QMessageBox::Yes | QMessageBox::No, QMessageBox::NoButton);
	if (answer == QMessageBox::Yes)
	{
		ui.CurveControlBox->setEnabled(false);
		ui.JogControlBox->setEnabled(true);
		ui.SelectControlModeButton->setText("ѡ�����ģʽ(�ٶȿ���)");
		myQPmac->closeLoop();
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
	ui.SelectControlModeButton->setText("ѡ�����ģʽ");
	ui.CurveControlBox->setEnabled(false);
	ui.JogControlBox->setEnabled(false);
	ui.InitPmacButton->setText("���ڳ�ʼ��...");
	//��ʼ��
	bool initState  = myQPmac->initPmac();
	QTimer::singleShot(1000, this, [=] 
	{
		qDebug() << "1s later:";
		//�������˲�
		if (initState)
		{
			myQPmac->enablePLC(1);
		}
		getPmacDataTimer->start();
		ui.SelectControlModeButton->setEnabled(true);

		ui.InitPmacButton->setText("��ʼ�����");;
	});





}
