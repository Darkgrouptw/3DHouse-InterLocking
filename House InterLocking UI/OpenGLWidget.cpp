#include "OpenGLWidget.h"

OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
}
OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::initializeGL()
{
	glClearColor(0, 0, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glLineWidth(1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	QMatrix4x4 pMatrix;
	QMatrix4x4 lookAt;
	pMatrix.perspective(60, 1, 0.1, 100);
	lookAt.lookAt(QVector3D(0, 10, 50),
		QVector3D(0, 10, 0),
		QVector3D(0, 1, 0));
	glMultMatrixf((pMatrix * lookAt).data());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OpenGLWidget::paintGL()
{
	if (chooseIndex == -1)
		return;

	for (int i = 0; i < pointDataArray[chooseIndex].size(); i++)
	{
		QVector<QVector<QVector3D>> objVertex = pointDataArray[chooseIndex][i];

		// µe QUADS
		glColor4f(0.81, 0.74, 0.33, 0.3);
		for (int j = 0; j < objVertex.size(); j++)
		{
			glBegin(GL_QUADS);
			for (int k = 0; k < objVertex[j].size(); k++)
				glVertex3f(objVertex[j][k].x(), objVertex[j][k].y(), objVertex[j][k].z());
			glEnd();
		}


		// µe LINE
		glColor4f(0, 0, 0, 1);
		glPolygonOffset(1.0, 1.0);
		for (int j = 0; j < objVertex.size(); j++)
		{
			glBegin(GL_LINES);
			for (int k = 0; k < objVertex[j].size(); k++)
			{
				int nextIndex = ((k + 1) == objVertex[j].size()) ? 0 : k + 1;
				glVertex3f(objVertex[j][k].x(), objVertex[j][k].y(), objVertex[j][k].z());
				glVertex3f(objVertex[j][nextIndex].x(), objVertex[j][nextIndex].y(), objVertex[j][nextIndex].z());
			}
			glEnd();
		}
	}


}
