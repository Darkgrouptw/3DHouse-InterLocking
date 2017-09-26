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

	// 只畫 Front Face
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}
void OpenGLWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// 設定矩陣
	SetPMVMatrix();

	// 左邊的視窗 (Model View)
	DrawHouseView();

	// 畫邊界
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
	#pragma region 地板
	info = new HouseTree();
	NodeInfo *ground = new NodeInfo;
	ground->nParams.XLength = 24;
	ground->nParams.YLength = 1;
	ground->nParams.ZLength = 16;
	ground->nParams.TranslatePoint = QVector3D(0, 0, 0);
	ground->name = "base/basic";
	info->Root = ground;
	#pragma endregion
	#pragma region 一個窗戶
	NodeInfo *single_window = new NodeInfo;
	single_window->nParams.XLength = 1;
	single_window->nParams.YLength = 10;
	single_window->nParams.ZLength = 15;
	single_window->nParams.TranslatePoint = QVector3D(-23, 11, 1);
	single_window->name = "wall/single_window";

	single_window->singleWindowParams.RatioWidth = 0.5;
	single_window->singleWindowParams.RatioHeight = 0.5;
	single_window->singleWindowParams.WindowWidth = 4;
	single_window->singleWindowParams.WindowHeight = 4;
	ground->childNode.push_back(single_window);
	#pragma endregion
	#pragma region 門
	NodeInfo *door_entry = new NodeInfo;
	door_entry->nParams.XLength = 1;
	door_entry->nParams.YLength = 10;
	door_entry->nParams.ZLength = 15;
	door_entry->nParams.TranslatePoint = QVector3D(23, 11, 1);
	
	door_entry->doorParams.ratio = 0.5;
	door_entry->doorParams.DoorWidth = 5;
	door_entry->doorParams.DoorHeight = 9;							// 從底下往上兩個高度 (所以離地面 DoorHeight x 2)
	door_entry->name = "wall/door_entry";
	ground->childNode.push_back(door_entry);
	#pragma endregion
}

void OpenGLWidget::SetPMVMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	QMatrix4x4 orthoMatrix;
	QMatrix4x4 lookAt;
	orthoMatrix.ortho(-30, 30, -5, 55, 0.1, 100);

	float LookPosX = 60.0f * qSin((float)AngleXZ / 180 * PI);
	float LookPosY = 5;
	float LookPosZ = 60.0f * qCos((float)AngleXZ / 180 * PI);
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
	#pragma region BFS 搜尋整棵樹 & 畫出整棟房子
	QVector<NodeInfo *> TreeTraverse;
	TreeTraverse.push_back(info->Root);
	while (TreeTraverse.size() > 0)
	{
		NodeInfo *currentNode = TreeTraverse[0];

		// 將物體轉成 3D 點，並劃出來
		QVector<QVector3D> ObjectPoint = TransformParamToModel(currentNode);
		DrawModelByName(currentNode->name, ObjectPoint);

		// 把全部的 child 放進來
		for (int i = 0; i < currentNode->childNode.size(); i++)
			TreeTraverse.push_back(currentNode->childNode[i]);

		// 把最前面的刪掉
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
		#pragma region 面
		#pragma region 前
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		tempArray.push_back(2);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 後
		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(7);
		tempArray.push_back(6);
		tempArray.push_back(5);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 上
		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(0);
		tempArray.push_back(3);
		tempArray.push_back(7);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 下
		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(5);
		tempArray.push_back(6);
		tempArray.push_back(2);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 左
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(4);
		tempArray.push_back(5);
		tempArray.push_back(1);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 右
		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(3);
		tempArray.push_back(2);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma endregion
		#pragma region 線
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(2);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(7);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(6);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(4);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(6);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(7);
		LineIndex.push_back(tempArray);
		#pragma endregion
	}
	else if (name == "wall/single_window")
	{
		#pragma region 面
		#pragma region 前
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		tempArray.push_back(5);
		tempArray.push_back(4);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(2);
		tempArray.push_back(6);
		tempArray.push_back(5);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(3);
		tempArray.push_back(7);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(0);
		tempArray.push_back(4);
		tempArray.push_back(7);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 後
		tempArray.clear();
		tempArray.push_back(8);
		tempArray.push_back(12);
		tempArray.push_back(13);
		tempArray.push_back(9);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(9);
		tempArray.push_back(13);
		tempArray.push_back(14);
		tempArray.push_back(10);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(10);
		tempArray.push_back(14);
		tempArray.push_back(15);
		tempArray.push_back(11);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(11);
		tempArray.push_back(15);
		tempArray.push_back(12);
		tempArray.push_back(8);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 上
		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(10);
		tempArray.push_back(11);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 下
		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(9);
		tempArray.push_back(8);
		tempArray.push_back(0);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 左
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		tempArray.push_back(11);
		tempArray.push_back(8);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 右
		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(9);
		tempArray.push_back(10);
		tempArray.push_back(2);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 內側
		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(5);
		tempArray.push_back(13);
		tempArray.push_back(12);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(6);
		tempArray.push_back(14);
		tempArray.push_back(13);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(7);
		tempArray.push_back(15);
		tempArray.push_back(14);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(12);
		tempArray.push_back(15);
		tempArray.push_back(7);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma endregion
		#pragma region 線
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(2);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(7);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(6);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);
		
		// 後方
		tempArray.clear();
		tempArray.push_back(8);
		tempArray.push_back(9);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(9);
		tempArray.push_back(10);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(10);
		tempArray.push_back(11);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(8);
		tempArray.push_back(11);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(15);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(15);
		tempArray.push_back(14);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(14);
		tempArray.push_back(13);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(13);
		LineIndex.push_back(tempArray);

		// 前方和後方
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(8);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(9);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(10);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(11);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(12);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(13);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(14);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(15);
		LineIndex.push_back(tempArray);
		#pragma endregion
	}
	else if (name == "wall/door_entry")
	{
		#pragma region 面
		#pragma region 前
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(4);
		tempArray.push_back(7);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(6);
		tempArray.push_back(2);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(1);
		tempArray.push_back(2);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 後
		tempArray.clear();
		tempArray.push_back(11);
		tempArray.push_back(15);
		tempArray.push_back(12);
		tempArray.push_back(8);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(11);
		tempArray.push_back(10);
		tempArray.push_back(14);
		tempArray.push_back(15);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(14);
		tempArray.push_back(10);
		tempArray.push_back(9);
		tempArray.push_back(13);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 上
		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(2);
		tempArray.push_back(10);
		tempArray.push_back(11);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 下
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(8);
		tempArray.push_back(12);
		tempArray.push_back(4);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(13);
		tempArray.push_back(9);
		tempArray.push_back(1);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 左
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		tempArray.push_back(11);
		tempArray.push_back(8);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 右
		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(9);
		tempArray.push_back(10);
		tempArray.push_back(2);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 內側
		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(6);
		tempArray.push_back(14);
		tempArray.push_back(13);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(7);
		tempArray.push_back(15);
		tempArray.push_back(14);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(12);
		tempArray.push_back(15);
		tempArray.push_back(7);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma endregion
		#pragma region 線
		// 前方
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(4);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(7);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(6);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(1);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(2);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(0);
		LineIndex.push_back(tempArray);

		// 後方
		tempArray.clear();
		tempArray.push_back(8);
		tempArray.push_back(12);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(15);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(15);
		tempArray.push_back(14);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(14);
		tempArray.push_back(13);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(13);
		tempArray.push_back(9);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(9);
		tempArray.push_back(10);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(10);
		tempArray.push_back(11);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(11);
		tempArray.push_back(8);
		LineIndex.push_back(tempArray);

		// 前方和後方
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(8);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(9);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(10);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(11);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(12);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(13);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(14);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(15);
		LineIndex.push_back(tempArray);
		#pragma endregion
	}
	else if (name == "wall/multi_window")
	{
	}

	#pragma region 根據 FaceIndex 去畫點
	glColor3f(0.81, 0.74, 0.33);
	for (int i = 0; i < FaceIndex.size(); i++)
	{
		glBegin(GL_QUADS);
		for (int j = 0; j < FaceIndex[i].size(); j++)
		{
			int index = FaceIndex[i][j];
			glVertex3f(pointData[index].x(), pointData[index].y(), pointData[index].z());
		}
		glEnd();
	}
	#pragma endregion
	#pragma region 根據 LineIndex 去線
	glColor3f(0, 0, 0);
	glPolygonOffset(2, 2);
	glBegin(GL_LINES);
	for (int i = 0; i < LineIndex.size(); i++)
		for (int j = 0; j < LineIndex[i].size(); j++)
		{
			int index = LineIndex[i][j];
			glVertex3f(pointData[index].x(), pointData[index].y(), pointData[index].z());
		}
	glEnd();
	#pragma endregion
}

QVector<QVector3D> OpenGLWidget::TransformParamToModel(NodeInfo *info)
{
	QVector<QVector3D> outputPoint;
	if (info->name == "base/basic")
	{
		NormalParams nParams = info->nParams;
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
	}
	else if (info->name == "wall/single_window")
	{
		NormalParams nParams = info->nParams;
		SingleWindowParams singleWindowParams = info->singleWindowParams;

		//////////////////////////////////////////////////////////////////////////
		// 從 -x 的地方看過去
		//////////////////////////////////////////////////////////////////////////

		float rz = qBound(-0.5f, singleWindowParams.RatioWidth - 0.5f, 0.5f);
		float ry = qBound(-0.5f, singleWindowParams.RatioHeight - 0.5f, 0.5f);

		float MoveableZ = qMax(nParams.ZLength - singleWindowParams.WindowWidth - 1, 0);
		float MoveableY = qMax(nParams.YLength - singleWindowParams.WindowHeight - 1, 0);

		// 最外側
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(-nParams.XLength, -singleWindowParams.WindowWidth + MoveableY * ry, -singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -singleWindowParams.WindowWidth + MoveableY * ry, singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, singleWindowParams.WindowWidth + MoveableY * ry, singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, singleWindowParams.WindowWidth + MoveableY * ry, -singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);

		// 內側
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(nParams.XLength, -singleWindowParams.WindowWidth + MoveableY * ry, -singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -singleWindowParams.WindowWidth + MoveableY * ry, singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, singleWindowParams.WindowWidth + MoveableY * ry, singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, singleWindowParams.WindowWidth + MoveableY * ry, -singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
	}
	else if (info->name == "wall/door_entry")
	{
		NormalParams nParams = info->nParams;
		DoorParams doorParams = info->doorParams;

		//////////////////////////////////////////////////////////////////////////
		// 從 x 的地方看過去
		//////////////////////////////////////////////////////////////////////////

		float r = qBound(-0.5f, doorParams.ratio - 0.5f, 0.5f);

		float Moveable = qMax(nParams.ZLength - doorParams.DoorWidth - 1, 1);

		// 最外側
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, -doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength + doorParams.DoorHeight * 2, -doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength + doorParams.DoorHeight * 2, doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);

		// 內側
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, -doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength + doorParams.DoorHeight * 2, -doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength + doorParams.DoorHeight * 2, doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
	}

	return outputPoint;
}
float OpenGLWidget::GetNextValue(float currentValue, float max)
{
	float nextValue = currentValue + 4;					// 下一個值

	// 超出的形況判斷
	if (nextValue >= max && currentValue < max)
		nextValue = max;

	// 判斷接下來的值，有沒有小於卡榫的洞的值，如果有就將 NextValue 切兩半
	if (abs(max - nextValue) > 0 && abs(max - nextValue) < 2)
		nextValue = (currentValue + max) / 2;
	return nextValue;
}
