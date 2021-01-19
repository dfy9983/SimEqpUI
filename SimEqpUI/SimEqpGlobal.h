#ifndef SIMEQPGLOBAL_H
#define SIMEQPGLOBAL_H
#include <iostream>
#include <QtCore>
#include <QDateTime>

class SimEqpGlobal;

class SimEqpGlobal
{
public:

	/*�㶯����*/
	static double jog_vel;//mm/s
	static double jog_disp;//mm

	/*��λ״̬*/
	static bool posLimState;//�� ����Ϊtrue
	static bool negLimState;//��

	/*������״̬*/
	static bool forceHomeState; //�������Ϊtrue
	static bool isHoming;//���ڻ���Ϊtrue

	/*�˶�ֹͣ״̬*/
	static bool isMoving;

	/*��ǰѡ�����ߵķ���*/
	static int windSpeed;//m/s


	/*��ǰʵʱ��λ�ơ��ٶ�����*/
	static double rt_disp;//mm
	static double rt_vel;//mm/s
	static double rt_force;//kg
	
	/*ʵʱ��ͼ���ʱ�� */
	static double intervalTime;
	static QDateTime startTime;
	static QDateTime endTime;

};

enum CurveType
{
	/*��������*/
	Disp,
	Vel,
	Force
};

#endif
