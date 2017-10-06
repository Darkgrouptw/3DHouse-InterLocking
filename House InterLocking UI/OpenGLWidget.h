#pragma once
#include <iostream>

#include <QComboBox>
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
	//////////////////////////////////////////////////////////////////////////
	// �O��!! �O�V�~�X�i!!
	//////////////////////////////////////////////////////////////////////////
	int XLength;													// X ����
	int YLength;													// Y ����
	int ZLength;													// Z ����

	QVector3D TranslatePoint;
};
struct DoorParams
{
	float ratio;

	int DoorWidth;
	int DoorHeight;
};
struct SingleWindowParams
{
	float RatioWidth;
	float RatioHeight;

	int WindowWidth;
	int WindowHeight;
};
struct MultiWindowParams
{
	SingleWindowParams windowA;
	SingleWindowParams windowB;
};
struct TriangleParams
{
	float ratioX;

	bool IsXY;
};
struct CrossGableParams
{
	float YOffset;
	float ZOffset;
};

struct NodeInfo
{
	QString name;													// ����W��

	// �Ѽ���
	NormalParams			nParams;
	SingleWindowParams		singleWindowParams;
	DoorParams				doorParams;
	MultiWindowParams		multiWindowParams;
	TriangleParams			triangleParams;
	CrossGableParams		gableParams;

	QVector<NodeInfo *>		childNode;								// �p�Ī� Node
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

	// ��ЫΪ����[�i���
	void AddComboBoxList();
	void UpdateByParams(int, int, int, float, float, float, float);

	QComboBox *comboBox = NULL;
	bool IsSetup = false;

	// �}�C��Info
	QVector<NodeInfo *> queueInfo;
private:
	//////////////////////////////////////////////////////////////////////////
	// �ƹ�����
	//////////////////////////////////////////////////////////////////////////
	int AngleXZ = 0;
	int LastAngleXZ = 0;
	bool IsLeftButtonClick = false;

	float LastRatioA;
	float LastRatioB;
	QPoint lastMousePosition;


	HouseTree *info;
	
	QVector<QVector3D> TransformParamToModel(NodeInfo *);			// �N�ѼƧ令�ҫ����I

	void InitModelParams();											// ��l�Ƽҫ��Ѽ�

	void SetPMVMatrix();											// �]�w�x�}
	void DrawHouseView();											// �e�X��ɩФl
	void DrawBorder();												// ���X�e�������
	void DrawModelByName(QString, QVector<QVector3D>);				// �ھڪ��骺�W�١A�٦��I�A�ӵe��

	float GetNextValue(float, float);
};