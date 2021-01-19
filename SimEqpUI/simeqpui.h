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

	/***************���߳�*******************/
	QThread *loadExcel_Qthread;
	ExcelThread *loadExcelThread;

	/************��ʱ��***********/
	QTimer *plotTimer;//��ʼʵʱ��ͼ��ʱ��
	QTimer *getPmacDataTimer;//��ȡPmac���ݶ�ʱ��
	QTimer *updateUiDataTimer;//ˢ��ui��ʱ��

	/**************����****************/
	/*ѡ�е����߻���*/
	QVector<QPen> selectedPen;
	/*ѡ�е�ͼ������*/
	QFont YHfontBold;
	/*λ��*/
	QCustomPlot *widget_Disp;
	CurvePlot *curves_Disp;
	/*�ٶ�*/
	QCustomPlot *widget_Vel;
	CurvePlot *curves_Vel;
	/*��*/
	QCustomPlot *widget_Force;
	CurvePlot *curves_Force;

	/*���ڶ�д�Ի���*/
	LoadExcelDlg * excelProssessDlg;

	/*���߲�������*/
	void addPoint(CurvePlot *graph, const double &x_temp, const double &y_temp);//������ݵ�
	void contextMenuRequestDisp(QPoint pos);//�Ҽ�ͼ��˵�_Dis
	void contextMenuRequestForce(QPoint pos);// �Ҽ�ͼ��˵�_Force
	void contextMenuRequestVel(QPoint pos);// �Ҽ�ͼ��˵�_Vel

private slots:
	void on_SelectControlModeButton_clicked(); //ѡ�����ģʽ
	void on_CurveMode_Force_triggered();//ѡ�������˶�ģʽ
	void on_JogMode_Vel_triggered();//ѡ��㶯�˶�ģʽ
	void on_ConnectPMACButton_clicked();//����PMAC
	void on_InitPmacButton_clicked();//��ʼ��PMAC
	//void on_MotorSevoOnButton_clicked();//����ŷ�on
	void on_ForceHomeButton_clicked();//������
	void on_StartButton_clicked();//��ʼ�˶� ����ʼ��ͼ
	void on_StopButton_clicked();//ֹͣ�˶�
	void on_RelDispJogButton_clicked();//����㶯
	void on_JogStopButton_clicked();//�㶯ֹͣ
	void on_LngPrsJogPosButton_pressed();//���� �����㶯 ���� 
	void on_LngPrsJogPosButton_released();//���� �����㶯 �ɿ� 
	void on_LngPrsJogNegButton_pressed();//���� �����㶯 ���� 
	void on_LngPrsJogNegButton_released();//���� �����㶯 �ɿ� 
	void on_ConfirmParaButton_clicked();//�㶯����ȷ��
	void on_JogVelEdit_textChanged();//�ٶȲ����仯����
	void on_JogDisEdit_textChanged();//��������仯����
	void on_plotRealTimeCurve_toggled(bool checked);//����ʵʱ���߱�ѡ��ʱ����
	void on_clearAllCurves_triggered();//������н�������

	void on_plotTimerTimeOut();//ʵʱ���ƶ�ʱ�����
	void on_getPmacDataTimer();//��ȡPMac���ݶ�ʱ�����
	void on_updateUiDataTimer();//ˢ�½������ݶ�ʱ�����

	void on_LoadDataButton_clicked();//�������excel���� ���س���PMAC
	void on_LoadCurveWinds10();//10m��������
	void on_LoadCurveWinds15();
	void on_LoadCurveWinds20();
	void on_LoadDispCurve(QString strFile);
	void on_LoadVelCurve(QString strFile);
	void on_LoadForceCurce(QString strFile);
	void readExcelFinished();//excel���ݶ�ȡ���

	void on_RecordDataButton_clicked();//����������excel
	void on_RecordDispCurve();
	void on_RecordForceCurce();
	void on_RecordVelCurce();
	void writeExcelFinished();//excel���ݶ�ȡ���
	void on_warnWriteExcelIsEmpty();//��������Ϊ�վ���

	void on_plotExcelData(CurveType curve);//����excel����
	void on_actZoomInToggled(bool arg1);//�����ѡ�Ŵ�
	void on_actMoveToggled(bool arg1);//����϶�
	void on_actSelectToggled(bool arg1);//ѡȡ��������

	void selectionChanged_Disp();//ͼ������ѡ��
	void selectionChanged_Force();//ͼ������ѡ��
	void selectionChanged_Vel();//ͼ������ѡ��


	void delay_block(int ms);//������ʱ����
	void delay_nonblock(int ms);//��������ʱ
};
