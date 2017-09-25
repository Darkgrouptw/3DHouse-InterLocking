#include "OpenGLWidget.h"

OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
	InitModelParams();
}
OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::initializeGL()
{
	glClearColor(0.69f, 0.93f, 0.93f, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glLineWidth(1);
}

void OpenGLWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// �]�w�x�}
	SetPMVMatrix();

	// ���䪺���� (Model View)
	DrawHouseView();

	// �e���
	DrawBorder();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *e)
{
	if (IsLeftButtonClick)
	{
		QPoint clampPos = QPoint(qBound(0, e->pos().x(), 600), qBound(0, e->pos().y(), 600));
		QPoint dis = clampPos - lastMousePosition;
		AngleXZ = (LastAngleXZ - dis.x() / 2 + 360) % 360;

		this->update();
	}
}
void OpenGLWidget::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
	{
		lastMousePosition = e->pos();
		IsLeftButtonClick = true;
		LastAngleXZ = AngleXZ;

		this->update();
	}
}
void OpenGLWidget::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
	{
		QPoint clampPos = QPoint(qBound(0, e->pos().x(), 600), qBound(0, e->pos().y(), 600));
		QPoint dis = clampPos - lastMousePosition;
		AngleXZ = (LastAngleXZ - dis.x() / 2 + 360) % 360;
		LastAngleXZ = AngleXZ;

		IsLeftButtonClick = false;
		this->update();
	}
}

void OpenGLWidget::InitModelParams()
{
	#pragma region �a�O
	info = new HouseTree();
	NodeInfo *ground = new NodeInfo;
	ground->nParams.Width = 24;
	ground->nParams.Height = 16;
	ground->nParams.Thinkness = 1;
	ground->nParams.TranslatePoint = QVector3D(0, 0, 0);
	ground->name = "base/basic";
	info->Root = ground;
	#pragma endregion
}

void OpenGLWidget::SetPMVMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	QMatrix4x4 orthoMatrix;
	QMatrix4x4 lookAt;
	orthoMatrix.ortho(-30, 30, -5, 55, 0.1, 100);

	float LookPosX = 20.0f * qSin((float)AngleXZ / 180 * PI);
	float LookPosY = 5;
	float LookPosZ = 20.0f * qCos((float)AngleXZ / 180 * PI);
	lookAt.lookAt(QVector3D(LookPosX, LookPosY, LookPosZ),
		QVector3D(0, 0, 0),
		QVector3D(0, 1, 0));

	glMultMatrixf((orthoMatrix * lookAt).data());

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OpenGLWidget::DrawHouseView()
{
	glViewport(0, 0, 599, 600);
	#pragma region BFS �j�M��ʾ� & �e�X��ɩФl
	QVector<NodeInfo *> TreeTraverse;
	TreeTraverse.push_back(info->Root);
	while (TreeTraverse.size() > 0)
	{
		NodeInfo *currentNode = TreeTraverse[0];

		// �N�����ন 3D �I�A�ù��X��
		QVector<QVector3D> ObjectPoint = TransformParamToModel(currentNode->nParams);
		DrawModelByName(currentNode->name, ObjectPoint);

		// ������� child ��i��
		for (int i = 0; i < currentNode->childNode.size(); i++)
			TreeTraverse.push_back(currentNode);

		// ��̫e�����R��
		TreeTraverse.pop_front();
	}
	#pragma endregion

}
void OpenGLWidget::DrawBorder()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(599, 0, 2, 600);

	glColor3f(0, 0, 0);
	glBegin(GL_QUADS);
	glVertex3f(-100, -100, 0);
	glVertex3f(100, -100, 0);
	glVertex3f(100, 100, 0);
	glVertex3f(-100, 100, 0);
	glEnd();
}

void OpenGLWidget::DrawModelByName(QString name, QVector<QVector3D> pointData)
{
	QVector<QVector<int>> FaceIndex;
	QVector<QVector<int>> LineIndex;

	QVector<int> tempArray;
	if (name == "base/basic")
	{
		#pragma region �e
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		tempArray.push_back(2);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region ��
		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(7);
		tempArray.push_back(6);
		tempArray.push_back(5);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region �W
		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(0);
		tempArray.push_back(3);
		tempArray.push_back(7);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region �U
		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(5);
		tempArray.push_back(6);
		tempArray.push_back(2);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region ��
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(4);
		tempArray.push_back(5);
		tempArray.push_back(1);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region �k
		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(3);
		tempArray.push_back(2);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		/*glPolygonOffset(2, 2);
		glColor3f(0, 0, 0);
		glBegin(GL_LINES);
		glVertex3f(pointData[0].x(), pointData[0].y(), pointData[0].z());
		glVertex3f(pointData[1].x(), pointData[1].y(), pointData[1].z());

		glVertex3f(pointData[1].x(), pointData[1].y(), pointData[1].z());
		glVertex3f(pointData[2].x(), pointData[2].y(), pointData[2].z());

		glVertex3f(pointData[2].x(), pointData[2].y(), pointData[2].z());
		glVertex3f(pointData[3].x(), pointData[3].y(), pointData[3].z());

		glVertex3f(pointData[3].x(), pointData[3].y(), pointData[3].z());

		glEnd();*/
	}

	#pragma region �ھ� FaceIndex �h�e�I
	glColor4f(0.81, 0.74, 0.33, 1);
	glBegin(GL_QUADS);
	for (int i = 0; i < FaceIndex.size(); i++)
		for (int j = 0; j < FaceIndex[i].size(); j++)
		{
			int index = FaceIndex[i][j];
			glVertex3f(pointData[index].x(), pointData[index].y(), pointData[index].z());
		}
	glEnd();
	#pragma endregion
}

QVector<QVector3D> OpenGLWidget::TransformParamToModel(NormalParams nParams)
{
	QVector<QVector3D> outputPoint;
	outputPoint.push_back(QVector3D(-nParams.Width, nParams.Thinkness, nParams.Height) + nParams.TranslatePoint);
	outputPoint.push_back(QVector3D(-nParams.Width, -nParams.Thinkness, nParams.Height) + nParams.TranslatePoint);
	outputPoint.push_back(QVector3D(nParams.Width, -nParams.Thinkness, nParams.Height) + nParams.TranslatePoint);
	outputPoint.push_back(QVector3D(nParams.Width, nParams.Thinkness, nParams.Height) + nParams.TranslatePoint);

	outputPoint.push_back(QVector3D(-nParams.Width, nParams.Thinkness, -nParams.Height) + nParams.TranslatePoint);
	outputPoint.push_back(QVector3D(-nParams.Width, -nParams.Thinkness, -nParams.Height) + nParams.TranslatePoint);
	outputPoint.push_back(QVector3D(nParams.Width, -nParams.Thinkness, -nParams.Height) + nParams.TranslatePoint);
	outputPoint.push_back(QVector3D(nParams.Width, nParams.Thinkness, -nParams.Height) + nParams.TranslatePoint);

	return outputPoint;
}

float OpenGLWidget::GetNextValue(float currentValue, float max)
{
	float nextValue = currentValue + 4;					// �U�@�ӭ�

	// �W�X���Ϊp�P�_
	if (nextValue >= max && currentValue < max)
		nextValue = max;

	// �P�_���U�Ӫ��ȡA���S���p��d�g���}���ȡA�p�G���N�N NextValue ����b
	if (abs(max - nextValue) > 0 && abs(max - nextValue) < 2)
		nextValue = (currentValue + max) / 2;
	return nextValue;
}
