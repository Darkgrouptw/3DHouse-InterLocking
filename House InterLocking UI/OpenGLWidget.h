#pragma once
#include <iostream>

#include <QtMath>
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLWidget>

#define PI 3.14159265

using namespace std;

struct NormalParams
{
	float Width;													// ��
	float Height;													// �e
	float Thinkness;												// �w

	QVector3D TranslatePoint;
};
struct NodeInfo
{
	QString name;													// ����W��
	NormalParams nParams;
	QVector<NodeInfo *> childNode;									// �p�Ī� Node
};
struct HouseTree
{
	NodeInfo *Root;
};
class OpenGLWidget: public QOpenGLWidget
{
public:
	OpenGLWidget(QWidget *);
	~OpenGLWidget();

	void initializeGL();
	void paintGL();
	

	// �ƹ��ƥ�
	void mouseMoveEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);

private:
	//////////////////////////////////////////////////////////////////////////
	// �ƹ�����
	//////////////////////////////////////////////////////////////////////////
	int AngleXZ = 0;
	int LastAngleXZ = 0;
	bool IsLeftButtonClick = false;
	QPoint lastMousePosition;


	HouseTree *info;
	QVector<QVector3D> TransformParamToModel(NormalParams);			// �N�ѼƧ令�ҫ����I

	void InitModelParams();											// ��l�Ƽҫ��Ѽ�

	void SetPMVMatrix();											// �]�w�x�}
	void DrawHouseView();											// �e�X��ɩФl
	void DrawBorder();												// ���X�e�������
	void DrawModelByName(QString, QVector<QVector3D>);				// �ھڪ��骺�W�١A�٦��I�A�ӵe��

	float GetNextValue(float, float);
};

