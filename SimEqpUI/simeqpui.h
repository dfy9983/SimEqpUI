#pragma once
#pragma execution_character_set("utf-8")
#include <QtWidgets/QMainWindow>
#include "ui_simeqpui.h"
#include "ExcelThread.h"
#include "CurvePlot.h"
#include "LoadExcelDlg.h"
#include "LoadExcelData.h"
#include <QAxObject>
#include "PmacDeviceLib.h"
#include "SimEqpGlobal.h"
#include "MyQPmac.h"
#include <QElapsedTimer>
class SimEqpUI : public QMainWindow
{
	Q_OBJECT

public:
	SimEqpUI(QWidget *parent = Q_NULLPTR);
	~SimEqpUI();
signals:
	void sig_ReadData(CurveType curve, QString fileName, QList<QList<QVariant>> *x_y0);
	void sig_WriteData(CurveType curve, QList<QList<QVariant> > *x_y, QString fileName);
	void sig_changeInitButton();

public slots:

private:
	Ui::SimEqpUIClass ui;
	void initUi();

	/*****************PMAC*******************/
	MyQPmac *myQPmac;

	/***************多线程*******************/
	QThread *loadExcel_Qthread;
	ExcelThread *loadExcelThread;

	/************定时器***********/
	QTimer *plotTimer;//开始实时绘图定时器
	QTimer *getPmacDataTimer;//获取Pmac数据定时器
	QTimer *updateUiDataTimer;//刷新ui定时器

	/**************曲线****************/
	/*选中的曲线画笔*/
	QVector<QPen> selectedPen;
	/*选中的图例字体*/
	QFont YHfontBold;
	/*位移*/
	QCustomPlot *widget_Disp;
	CurvePlot *curves_Disp;
	/*速度*/
	QCustomPlot *widget_Vel;
	CurvePlot *curves_Vel;
	/*力*/
	QCustomPlot *widget_Force;
	CurvePlot *curves_Force;

	/*正在读写对话框*/
	LoadExcelDlg * excelProssessDlg;

	/*曲线操作绘制*/
	void addPoint(CurvePlot *graph, const double &x_temp, const double &y_temp);//添加数据点
	void contextMenuRequestDisp(QPoint pos);//右键图像菜单_Dis
	void contextMenuRequestForce(QPoint pos);// 右键图像菜单_Force
	void contextMenuRequestVel(QPoint pos);// 右键图像菜单_Vel

private slots:
	void on_SelectControlModeButton_clicked(); //选择控制模式
	void on_CurveMode_Force_triggered();//选择曲线运动模式
	void on_JogMode_Vel_triggered();//选择点动运动模式
	void on_ConnectPMACButton_clicked();//连接PMAC
	void on_InitPmacButton_clicked();//初始化PMAC
	void on_MotorSevoOnButton_clicked();//电机伺服on
	void on_ForceHomeButton_clicked();//力回零
	void on_StartButton_clicked();//开始运动 并开始绘图
	void on_StopButton_clicked();//停止运动
	void on_RelDispJogButton_clicked();//距离点动
	void on_ConfirmParaButton_clicked();//点动参数确认
	void on_JogVelEdit_textChanged();//参数变化触发
	void on_JogDisEdit_textChanged();//参数变化触发
	void on_plotRealTimeCurve_toggled(bool checked);//绘制实时曲线被选择时触发
	void on_clearAllCurves_triggered();//清除所有曲线

	void on_plotTimerTimeOut();//实时绘制定时器溢出
	void on_getPmacDataTimer();//获取PMac数据定时器溢出
	void on_updateUiDataTimer();//刷新界面数据定时器溢出

	void on_LoadDataButton_clicked();//导入加载excel数据 下载程序到PMAC
	void on_LoadCurveWinds10();//10m风速曲线
	void on_LoadCurveWinds15();
	void on_LoadCurveWinds20();
	void on_LoadDispCurve(QString strFile);
	void on_LoadVelCurve(QString strFile);
	void on_LoadForceCurce(QString strFile);
	void readExcelFinished();//excel数据读取完成

	void on_RecordDataButton_clicked();//保存数据至excel
	void on_RecordDispCurve();
	void on_RecordForceCurce();
	void on_RecordVelCurce();
	void writeExcelFinished();//excel数据读取完成
	void on_warnWriteExcelIsEmpty();//保存数据为空警告

	void on_plotExcelData(CurveType curve);//绘制excel曲线
	void on_actZoomInToggled(bool arg1);//左键框选放大
	void on_actMoveToggled(bool arg1);//左键拖动
	void on_actSelectToggled(bool arg1);//选取曲线数据

	void selectionChanged_Disp();//图例曲线选择
	void selectionChanged_Force();//图例曲线选择
	void selectionChanged_Vel();//图例曲线选择


	void delay_block(int ms);//阻塞延时程序
	void delay_nonblock(int ms);//非阻塞延时
};
