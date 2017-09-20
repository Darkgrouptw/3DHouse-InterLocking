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
	第一層 => model_part
	第二層 => 幾個 obj
	第三層 => 裡面有幾個面
	第四層 => 每個面有幾個點
	*/

	int chooseIndex = -1;
};

