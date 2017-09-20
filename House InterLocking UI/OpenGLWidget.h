#pragma once
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLWidget>
class OpenGLWidget: public QOpenGLWidget
{
public:
	OpenGLWidget(QWidget *);
	~OpenGLWidget();

	void initializeGL();
	void paintGL();

	QVector<QVector<QVector<QVector<QVector3D>>>> pointDataArray;
	/*
	�Ĥ@�h => model_part
	�ĤG�h => �X�� obj
	�ĤT�h => �̭����X�ӭ�
	�ĥ|�h => �C�ӭ����X���I
	*/

	int chooseIndex = -1;
};

