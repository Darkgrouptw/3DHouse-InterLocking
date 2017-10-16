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

	if (comboBox != NULL && !IsSetup)
	{
		AddComboBoxList();
		IsSetup = true;
	}
	
	// 設定矩陣，左邊的視窗 (Model View)
	SetPMVMatrix();
	DrawHouseView();

	// 畫邊界
	DrawBorder();

	// 畫右邊的結果
	SetPMVMatrix();
	DrawSliceResult();
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
		lastMousePosition = QPoint(qBound(0, e->pos().x(), 600), qBound(0, e->pos().y(), 600));
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

void OpenGLWidget::AddComboBoxList()
{
	for (int i = 0; i < queueInfo.size(); i++)
		comboBox->addItem(queueInfo[i]->name);
}

void OpenGLWidget::UpdateByParams(int index, int width, int height, float ratioAW, float ratioAH, float ratioBW, float ratioBH)
{
	#pragma region 根據 Case 改 Params
	switch (index)
	{
	case 0:
		{
		// 最大可以縮減的範圍到哪(也就是牆)
		int MinWidth = -1;
		int MinHeight = -1;

		#pragma region 拿地板 & 窗戶的資訊，判斷最小的區間在哪裡
		for (int i = 0; i < queueInfo.size(); i++)
		{
			// 窗戶的話，是要比 ZLength
			if (queueInfo[i]->name == "wall/single_window")
				MinHeight = qMax(MinHeight, queueInfo[i]->singleWindowParams.WindowWidth + 2);
			else if (queueInfo[i]->name == "wall/door_entry")
				MinHeight = qMax(MinHeight, queueInfo[i]->doorParams.DoorWidth + 2);
			else if (queueInfo[i]->name == "wall/multi_window")
			{
				float RatioGap = qAbs(queueInfo[i]->multiWindowParams.windowA.RatioWidth - queueInfo[i]->multiWindowParams.windowB.RatioWidth);
				int actuallyLength = RatioGap * queueInfo[i]->nParams.XLength;
				MinWidth = qMax(MinWidth, actuallyLength + queueInfo[i]->multiWindowParams.windowA.WindowWidth / 2 + queueInfo[i]->multiWindowParams.windowB.WindowWidth / 2 + 2);

			}
		}
		#pragma endregion
		
		queueInfo[index]->nParams.XLength = qMax(width, MinWidth);
		queueInfo[index]->nParams.ZLength = qMax(height, MinHeight);
		break;
		}
	case 1:
		queueInfo[index]->singleWindowParams.RatioWidth = qBound(0.0f, ratioAW, 1.0f);
		queueInfo[index]->singleWindowParams.RatioHeight = qBound(0.0f, ratioAH, 1.0f);
		break;
	case 2:
		queueInfo[index]->doorParams.ratio = qBound(0.0f, ratioAW, 1.0f);
		break;
	case 3:
		queueInfo[index]->multiWindowParams.windowA.RatioWidth = qBound(0.0f, ratioAW, 1.0f);
		queueInfo[index]->multiWindowParams.windowA.RatioHeight = qBound(0.0f, ratioAH, 1.0f);
		queueInfo[index]->multiWindowParams.windowB.RatioWidth = qBound(0.0f, ratioBW, 1.0f);
		queueInfo[index]->multiWindowParams.windowB.RatioHeight = qBound(0.0f, ratioBH, 1.0f);
		break;
	case 6:
		queueInfo[index]->gableParams.ratio = 1 - qBound(0.0f, ratioAW, 1.0f);
		break;
	}
	#pragma endregion
	#pragma region 第一個規則 => Parent 都是比 Child 大
	QVector<NodeInfo *> infoArray;
	infoArray.push_back(info->Root);

	while (infoArray.size() > 0)
	{
		NodeInfo *currentInfo = infoArray[0];
		
		// 要判斷是不是在範圍內
		QVector3D pos = currentInfo->nParams.TranslatePoint;
		for (int i = 0; i < currentInfo->childNode.size(); i++)
		{
			NodeInfo *childInfo = currentInfo->childNode[i];
			QVector3D& childPos = childInfo->nParams.TranslatePoint;

			if (childInfo->name == "roof/cross_gable")
			{
				childInfo->nParams.XLength = qBound(0, childInfo->nParams.XLength, currentInfo->nParams.XLength);
				childInfo->nParams.ZLength = qBound(0, childInfo->nParams.ZLength, currentInfo->nParams.ZLength);
			}
			else if (childInfo->name == "wall/multi_window")
			{
				childInfo->nParams.XLength = qBound(0, childInfo->nParams.XLength, currentInfo->nParams.XLength);
				childPos.setZ(qBound(pos.z() - currentInfo->nParams.ZLength + childInfo->nParams.ZLength, childPos.z(), pos.z() + currentInfo->nParams.ZLength - childInfo->nParams.ZLength));
			}
			else if (childInfo->name == "wall/single_window" || childInfo->name == "wall/door_entry")
			{
				childPos.setX(qBound(pos.x() - currentInfo->nParams.XLength + childInfo->nParams.XLength, childPos.x(), pos.x() + currentInfo->nParams.XLength - childInfo->nParams.XLength));
				childInfo->nParams.ZLength = qBound(0, childInfo->nParams.ZLength, currentInfo->nParams.ZLength -1);		// 因為雙窗的厚度為 1，所以後面才要扣1
				
				// 假設厚度固定為 0
				childPos.setZ(qBound(pos.z() - currentInfo->nParams.ZLength + childInfo->nParams.ZLength, childPos.z(), pos.z() + currentInfo->nParams.ZLength - childInfo->nParams.ZLength));
			}
			else if (childInfo->name == "roof/Triangle")
			{
				// 三角形跟上面的 Case 很像，唯一不一樣的地方在於，他可以多一個後牆的寬度
				childPos.setX(qBound(pos.x() - currentInfo->nParams.XLength + childInfo->nParams.XLength, childPos.x(), pos.x() + currentInfo->nParams.XLength - childInfo->nParams.XLength));
				childInfo->nParams.ZLength = qBound(1, childInfo->nParams.ZLength, currentInfo->nParams.ZLength + 1);		// 三角形因為是在 門 和 窗之下，所以沒算到後牆的 1
				
				// 因為有後牆的厚度，所以要位移兩個單位(厚度唯一，但是往外長兩個單位)
				childPos.setZ(qBound(pos.z() - currentInfo->nParams.ZLength + childInfo->nParams.ZLength - 2, childPos.z(), pos.z() + currentInfo->nParams.ZLength - childInfo->nParams.ZLength - 2));
			}

			// 加上 Child 的，把它全部丟到陣列裡
			infoArray.push_back(currentInfo->childNode[i]);
		}

		// 把最前面的 POP 掉
		infoArray.pop_front();
	}
	#pragma endregion
	#pragma region 第二個規則 => 多窗不能卡到
	if (queueInfo[index]->name == "wall/multi_window")
	{
		NodeInfo* MultiWindowInfo = queueInfo[index];
		float GapLength = MultiWindowInfo->multiWindowParams.windowA.WindowWidth + MultiWindowInfo->multiWindowParams.windowB.WindowWidth + 1;
		float GapRatio = GapLength / MultiWindowInfo->nParams.XLength / 2;

		float RatioBorderA = (float)(MultiWindowInfo->multiWindowParams.windowA.WindowWidth + 1) / MultiWindowInfo->nParams.XLength / 2;
		float RatioBorderB = (float)(MultiWindowInfo->multiWindowParams.windowB.WindowWidth + 1) / MultiWindowInfo->nParams.XLength / 2;

		//////////////////////////////////////////////////////////////////////////
		// 改兩個窗的 Ratio
		// 1. 判斷是否有交錯
		// 2. 判斷是否撞到左牆
		// 3. 判斷是否撞到右牆
		// 4. 不能超過右牆
		//////////////////////////////////////////////////////////////////////////
		if (qAbs(MultiWindowInfo->multiWindowParams.windowA.RatioWidth - MultiWindowInfo->multiWindowParams.windowB.RatioWidth) < GapRatio ||
			(MultiWindowInfo->multiWindowParams.windowA.RatioWidth - RatioBorderA <= 0 || MultiWindowInfo->multiWindowParams.windowB.RatioWidth - RatioBorderB <= 0) ||
			(MultiWindowInfo->multiWindowParams.windowA.RatioWidth + RatioBorderA >= 1 || MultiWindowInfo->multiWindowParams.windowB.RatioWidth + RatioBorderB >= 1) ||
			MultiWindowInfo->multiWindowParams.windowA.RatioWidth >= MultiWindowInfo->multiWindowParams.windowB.RatioWidth)
		{
			// 改成以前的 Ratio
			MultiWindowInfo->multiWindowParams.windowA.RatioWidth = LastRatioA;
			MultiWindowInfo->multiWindowParams.windowB.RatioWidth = LastRatioB;
		}
		else
		{
			LastRatioA = MultiWindowInfo->multiWindowParams.windowA.RatioWidth;
			LastRatioB = MultiWindowInfo->multiWindowParams.windowB.RatioWidth;
		}

		// 改長寬
		MultiWindowInfo->multiWindowParams.windowA.WindowWidth = qBound(1, width, 8);
		MultiWindowInfo->multiWindowParams.windowA.WindowHeight = qBound(1, height, MultiWindowInfo->nParams.YLength - 2);
	}
	#pragma endregion
	#pragma region 第三個規則 => 窗戶不能超過原本大小
	if (queueInfo[index]->name == "roof/cross_gable")
	{
		NodeInfo* Gable = queueInfo[index];
		float CurrentPosX = (Gable->gableParams.ratio - 0.5) * 2 * Gable->nParams.XLength;
		cout << "Current " << CurrentPosX << endl;

		// 先壓縮到可以的範圍內
		width = qBound(1, width ,Gable->nParams.XLength - 1);
		height = qBound(1, height, Gable->nParams.YLength - 1);
		if (CurrentPosX + Gable->gableParams.ConvexWidth + 1 > Gable->nParams.XLength)
		{
			float PassCenterPos = Gable->nParams.XLength - 1 - width;
			Gable->gableParams.ratio = PassCenterPos / Gable->nParams.XLength / 2 + 0.5f;
		}
		else if (CurrentPosX - Gable->gableParams.ConvexWidth - 1 < -Gable->nParams.XLength)
		{
			float PassCenterPos = -Gable->nParams.XLength + 1 + width;
			Gable->gableParams.ratio = PassCenterPos / Gable->nParams.XLength / 2 + 0.5f;
		}
		Gable->gableParams.ConvexWidth = width;
		Gable->gableParams.ConvexHeight = height;
	}
	#pragma endregion
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

	queueInfo.push_back(ground);
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

	queueInfo.push_back(single_window);
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

	queueInfo.push_back(door_entry);
	#pragma endregion
	#pragma region 兩個窗
	NodeInfo *multiWindow = new NodeInfo;
	multiWindow->nParams.XLength = 24;
	multiWindow->nParams.YLength = 10;
	multiWindow->nParams.ZLength = 1;
	multiWindow->nParams.TranslatePoint = QVector3D(0, 11, -15);
	multiWindow->name = "wall/multi_window";

	multiWindow->multiWindowParams.windowA.RatioWidth = 0.2;
	multiWindow->multiWindowParams.windowA.RatioHeight = 0.5;
	multiWindow->multiWindowParams.windowA.WindowWidth = 4;
	multiWindow->multiWindowParams.windowA.WindowHeight = 4;
	multiWindow->multiWindowParams.windowB.RatioWidth = 0.8;
	multiWindow->multiWindowParams.windowB.RatioHeight = 0.5;
	multiWindow->multiWindowParams.windowB.WindowWidth = 4;
	multiWindow->multiWindowParams.windowB.WindowHeight = 4;
	ground->childNode.push_back(multiWindow);

	queueInfo.push_back(multiWindow);
	#pragma endregion
	#pragma region 三角形
	NodeInfo *roof_leftTriangle = new NodeInfo;
	roof_leftTriangle->nParams.XLength = 1;
	roof_leftTriangle->nParams.YLength = 10;								// 從底往上YLength
	roof_leftTriangle->nParams.ZLength = 16;
	roof_leftTriangle->nParams.TranslatePoint = QVector3D(-23, 21, 0);
	roof_leftTriangle->name = "roof/Triangle";

	roof_leftTriangle->triangleParams.ratioX = 0.5;
	single_window->childNode.push_back(roof_leftTriangle);

	queueInfo.push_back(roof_leftTriangle);

	roof_leftTriangle = new NodeInfo;
	roof_leftTriangle->nParams.XLength = 1;
	roof_leftTriangle->nParams.YLength = 10;								// 從底往上YLength
	roof_leftTriangle->nParams.ZLength = 16;
	roof_leftTriangle->nParams.TranslatePoint = QVector3D(23, 21, 0);
	roof_leftTriangle->name = "roof/Triangle";

	roof_leftTriangle->triangleParams.ratioX = 0.5;
	door_entry->childNode.push_back(roof_leftTriangle);

	queueInfo.push_back(roof_leftTriangle);
	#pragma endregion
	#pragma region 屋頂
	NodeInfo *gable = new NodeInfo;
	gable->nParams.XLength = 24;
	gable->nParams.YLength = 10;
	gable->nParams.ZLength = 16;
	gable->nParams.TranslatePoint = QVector3D(0, 21, 0);
	gable->name = "roof/cross_gable";

	gable->gableParams.YOffset = 0.625;
	gable->gableParams.ZOffset = 1;
	gable->gableParams.ratio = 0.5;
	gable->gableParams.ConvexWidth = 6.44;
	gable->gableParams.ConvexHeight = 9;

	ground->childNode.push_back(gable);

	queueInfo.push_back(gable);
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
void OpenGLWidget::DrawSliceResult()
{
	glViewport(601, 0, 599, 600);
	#pragma region BFS 搜尋整棵樹 & 畫出整棟房子
	QVector<NodeInfo *> TreeTraverse;
	TreeTraverse.push_back(info->Root);
	while (TreeTraverse.size() > 0)
	{
		NodeInfo *currentNode = TreeTraverse[0];

		// 將物體轉成 3D 點，並劃出來
		QVector<QVector3D> ObjectPoint = TransformParamToModel(currentNode);
		DrawSliceModelByName(currentNode->name, ObjectPoint);

		// 把全部的 child 放進來
		for (int i = 0; i < currentNode->childNode.size(); i++)
			TreeTraverse.push_back(currentNode->childNode[i]);

		// 把最前面的刪掉
		TreeTraverse.pop_front();
	}
	#pragma endregion
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
		tempArray.push_back(9);
		tempArray.push_back(8);
		tempArray.push_back(5);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(2);
		tempArray.push_back(10);
		tempArray.push_back(9);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(6);
		tempArray.push_back(11);
		tempArray.push_back(10);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(3);
		tempArray.push_back(7);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(8);
		tempArray.push_back(11);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(3);
		tempArray.push_back(0);
		tempArray.push_back(4);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 後
		tempArray.clear();
		tempArray.push_back(16);
		tempArray.push_back(17);
		tempArray.push_back(13);
		tempArray.push_back(12);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(17);
		tempArray.push_back(20);
		tempArray.push_back(21);
		tempArray.push_back(13);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(21);
		tempArray.push_back(22);
		tempArray.push_back(14);
		tempArray.push_back(13);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(22);
		tempArray.push_back(23);
		tempArray.push_back(18);
		tempArray.push_back(14);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(18);
		tempArray.push_back(19);
		tempArray.push_back(15);
		tempArray.push_back(14);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(18);
		tempArray.push_back(23);
		tempArray.push_back(20);
		tempArray.push_back(17);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(16);
		tempArray.push_back(12);
		tempArray.push_back(15);
		tempArray.push_back(19);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 側
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(12);
		tempArray.push_back(13);
		tempArray.push_back(1);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(1);
		tempArray.push_back(13);
		tempArray.push_back(14);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(2);
		tempArray.push_back(14);
		tempArray.push_back(15);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		tempArray.push_back(15);
		tempArray.push_back(12);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(4);
		tempArray.push_back(16);
		tempArray.push_back(19);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(5);
		tempArray.push_back(17);
		tempArray.push_back(16);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(6);
		tempArray.push_back(18);
		tempArray.push_back(17);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(7);
		tempArray.push_back(19);
		tempArray.push_back(18);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(11);
		tempArray.push_back(8);
		tempArray.push_back(20);
		tempArray.push_back(23);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(8);
		tempArray.push_back(9);
		tempArray.push_back(21);
		tempArray.push_back(20);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(9);
		tempArray.push_back(10);
		tempArray.push_back(22);
		tempArray.push_back(21);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(10);
		tempArray.push_back(11);
		tempArray.push_back(23);
		tempArray.push_back(22);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma endregion
		#pragma region 線
		#pragma region 前
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
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(6);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(7);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(4);
		LineIndex.push_back(tempArray);

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
		tempArray.push_back(11);
		tempArray.push_back(8);
		LineIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 後
		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(13);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(13);
		tempArray.push_back(14);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(14);
		tempArray.push_back(15);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(15);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(16);
		tempArray.push_back(17);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(17);
		tempArray.push_back(18);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(18);
		tempArray.push_back(19);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(19);
		tempArray.push_back(16);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(20);
		tempArray.push_back(21);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(21);
		tempArray.push_back(22);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(22);
		tempArray.push_back(23);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(23);
		tempArray.push_back(20);
		LineIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 側
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(12);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(13);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(14);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(15);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(16);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(17);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(18);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(19);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(8);
		tempArray.push_back(20);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(9);
		tempArray.push_back(21);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(10);
		tempArray.push_back(22);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(11);
		tempArray.push_back(23);
		LineIndex.push_back(tempArray);
		#pragma endregion
		#pragma endregion
	}
	else if (name == "roof/Triangle")
	{
		#pragma region 面
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		tempArray.push_back(2);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(3);
		tempArray.push_back(5);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		tempArray.push_back(4);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(2);
		tempArray.push_back(5);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(1);
		tempArray.push_back(4);
		tempArray.push_back(5);
		FaceIndex.push_back(tempArray);
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
		tempArray.push_back(0);
		tempArray.push_back(2);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(4);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		// 側面
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(4);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);
		#pragma endregion
	}
	else if (name == "roof/cross_gable")
	{
		#pragma region 面
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		tempArray.push_back(4);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(2);
		tempArray.push_back(5);
		tempArray.push_back(4);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(9);
		tempArray.push_back(10);
		tempArray.push_back(7);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(10);
		tempArray.push_back(11);
		tempArray.push_back(8);
		tempArray.push_back(7);
		FaceIndex.push_back(tempArray);

		// 側面
		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(5);
		tempArray.push_back(11);
		tempArray.push_back(10);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		tempArray.push_back(9);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(1);
		tempArray.push_back(7);
		tempArray.push_back(8);
		FaceIndex.push_back(tempArray);

		// 凸之前的地方
		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(13);
		tempArray.push_back(12);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(15);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(13);
		tempArray.push_back(10);
		tempArray.push_back(9);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(14);
		tempArray.push_back(12);
		tempArray.push_back(9);
		FaceIndex.push_back(tempArray);

		// 凸起來的地方
		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(14);
		tempArray.push_back(16);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(15);
		tempArray.push_back(12);
		tempArray.push_back(16);
		FaceIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(14);
		tempArray.push_back(15);
		tempArray.push_back(16);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region 線
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(4);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(1);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(5);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(4);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(7);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(10);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(10);
		tempArray.push_back(9);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(6);
		tempArray.push_back(9);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(8);
		tempArray.push_back(7);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(8);
		tempArray.push_back(11);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(10);
		tempArray.push_back(11);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(7);
		tempArray.push_back(10);
		LineIndex.push_back(tempArray);

		// 側邊
		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(7);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(8);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(10);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(11);
		LineIndex.push_back(tempArray);

		// 凸之前的部分
		tempArray.clear();
		tempArray.push_back(9);
		tempArray.push_back(14);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(14);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(15);
		tempArray.push_back(3);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(15);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(13);
		tempArray.push_back(12);
		LineIndex.push_back(tempArray);

		// 凸之後的部分
		tempArray.clear();
		tempArray.push_back(12);
		tempArray.push_back(16);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(14);
		tempArray.push_back(16);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(15);
		tempArray.push_back(16);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(14);
		tempArray.push_back(15);
		LineIndex.push_back(tempArray);
		#pragma endregion
	}

	#pragma region 根據 FaceIndex 去畫點
	glColor3f(0.81, 0.74, 0.33);
	for (int i = 0; i < FaceIndex.size(); i++)
	{
		if (FaceIndex[i].size()== 3)
			glBegin(GL_TRIANGLES);
		else
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
void OpenGLWidget::DrawSliceModelByName(QString name, QVector<QVector3D> pointData)
{
	// 要先畫出邊框
	DrawModelByName(name, pointData);

	//接下來再根據每個點，去畫切出來的結果
	QVector<QVector3D> startVector;
	QVector<QVector3D> endVector;

	if (name == "base/basic")
	{
		// 上
		startVector.push_back(pointData[4]);
		endVector.push_back(pointData[3]);

		// 下
		startVector.push_back(pointData[5]);
		endVector.push_back(pointData[2]);

		//左
		startVector.push_back(pointData[5]);
		endVector.push_back(pointData[0]);

		//右
		startVector.push_back(pointData[6]);
		endVector.push_back(pointData[3]);

		// 前
		startVector.push_back(pointData[1]);
		endVector.push_back(pointData[3]);

		// 後
		startVector.push_back(pointData[5]);
		endVector.push_back(pointData[7]);
	}
	else if (name == "wall/single_window")
	{
		// 前
		startVector.push_back(pointData[0]);																// 第一部分
		endVector.push_back(QVector3D(pointData[7].x(), pointData[3].y(), pointData[7].z()));
		startVector.push_back(QVector3D(pointData[4].x(), pointData[0].y(), pointData[4].z()));				// 第二部分
		endVector.push_back(pointData[5]);
		startVector.push_back(QVector3D(pointData[7].x(), pointData[3].y(), pointData[7].z()));				// 第三部分
		endVector.push_back(pointData[6]);
		startVector.push_back(QVector3D(pointData[5].x(), pointData[1].y(), pointData[5].z()));				// 第四部份
		endVector.push_back(pointData[2]);

		// 後
		startVector.push_back(pointData[8]);																// 第一部分
		endVector.push_back(QVector3D(pointData[15].x(), pointData[11].y(), pointData[15].z()));
		startVector.push_back(QVector3D(pointData[12].x(), pointData[8].y(), pointData[12].z()));			// 第二部分
		endVector.push_back(pointData[13]);
		startVector.push_back(QVector3D(pointData[15].x(), pointData[11].y(), pointData[15].z()));			// 第三部分
		endVector.push_back(pointData[14]);
		startVector.push_back(QVector3D(pointData[13].x(), pointData[9].y(), pointData[13].z()));			// 第四部份
		endVector.push_back(pointData[10]);

		// 前
		startVector.push_back(pointData[1]);
		endVector.push_back(pointData[10]);
	}
	else if (name == "wall/door_entry")
	{
		// 前
		startVector.push_back(pointData[0]);																// 第一部分
		endVector.push_back(QVector3D(pointData[7].x(), pointData[3].y(), pointData[7].z()));
		startVector.push_back(QVector3D(pointData[7].x(), pointData[3].y(), pointData[7].z()));				// 第二部分
		endVector.push_back(pointData[6]);
		startVector.push_back(pointData[5]);																// 第三部份
		endVector.push_back(pointData[2]);

		// 後
		startVector.push_back(pointData[8]);																// 第一部分
		endVector.push_back(QVector3D(pointData[15].x(), pointData[11].y(), pointData[15].z()));
		startVector.push_back(QVector3D(pointData[15].x(), pointData[11].y(), pointData[15].z()));			// 第二部分
		endVector.push_back(pointData[14]);
		startVector.push_back(pointData[13]);																// 第三部份
		endVector.push_back(pointData[10]);

		// 前
		startVector.push_back(pointData[0]);
		endVector.push_back(pointData[11]);
	}
	else if (name == "wall/multi_window")
	{
		// 前
		startVector.push_back(pointData[0]);																// 第一部分
		endVector.push_back(QVector3D(pointData[4].x(), pointData[3].y(), pointData[4].z()));
		startVector.push_back(QVector3D(pointData[4].x(), pointData[0].y(), pointData[4].z()));				// 第二部分
		endVector.push_back(pointData[5]);
		startVector.push_back(QVector3D(pointData[7].x(), pointData[3].y(), pointData[7].z()));				// 第三部分
		endVector.push_back(pointData[6]);
		startVector.push_back(QVector3D(pointData[5].x(), pointData[0].y(), pointData[5].z()));				// 第四部份
		endVector.push_back(QVector3D(pointData[11].x(), pointData[3].y(), pointData[11].z()));
		startVector.push_back(QVector3D(pointData[8].x(), pointData[0].y(), pointData[8].z()));				// 第五部份
		endVector.push_back(pointData[9]);
		startVector.push_back(QVector3D(pointData[11].x(), pointData[2].y(), pointData[11].z()));			// 第六部份
		endVector.push_back(pointData[10]);
		startVector.push_back(QVector3D(pointData[9].x(), pointData[1].y(), pointData[2].z()));				// 第七部份
		endVector.push_back(pointData[2]);

		// 後
		startVector.push_back(pointData[12]);																// 第一部分
		endVector.push_back(QVector3D(pointData[16].x(), pointData[15].y(), pointData[16].z()));
		startVector.push_back(QVector3D(pointData[16].x(), pointData[12].y(), pointData[16].z()));			// 第二部分
		endVector.push_back(pointData[17]);
		startVector.push_back(QVector3D(pointData[19].x(), pointData[15].y(), pointData[19].z()));			// 第三部分
		endVector.push_back(pointData[18]);
		startVector.push_back(QVector3D(pointData[17].x(), pointData[12].y(), pointData[17].z()));			// 第四部份
		endVector.push_back(QVector3D(pointData[23].x(), pointData[15].y(), pointData[23].z()));
		startVector.push_back(QVector3D(pointData[20].x(), pointData[12].y(), pointData[20].z()));			// 第五部份
		endVector.push_back(pointData[21]);
		startVector.push_back(QVector3D(pointData[23].x(), pointData[14].y(), pointData[23].z()));			// 第六部份
		endVector.push_back(pointData[22]);
		startVector.push_back(QVector3D(pointData[21].x(), pointData[13].y(), pointData[14].z()));			// 第七部份
		endVector.push_back(pointData[14]);

		// 左
		startVector.push_back(pointData[0]);
		endVector.push_back(pointData[15]);

		// 右
		startVector.push_back(pointData[1]);
		endVector.push_back(pointData[14]);
	}
	else if (name == "roof/Triangle")
	{
		// 左側
		startVector.push_back(pointData[0]);
		endVector.push_back(pointData[2]);
		startVector.push_back(pointData[1]);
		endVector.push_back(pointData[2]);

		// 右側
		startVector.push_back(pointData[3]);
		endVector.push_back(pointData[5]);
		startVector.push_back(pointData[4]);
		endVector.push_back(pointData[5]);
	}
	else if (name == "roof/cross_gable")
	{
		// 正面
		startVector.push_back(pointData[5]);
		endVector.push_back(pointData[10]);

		// 凸之前的部分
		startVector.push_back(pointData[3]);
		endVector.push_back(pointData[13]);
		startVector.push_back(pointData[9]);
		endVector.push_back(pointData[13]);

		#pragma region 凸出來的部分
		glPointSize(10);
		glBegin(GL_LINES);
		glVertex3f(pointData[16].x(), pointData[16].y(), pointData[16].z());
		glVertex3f(pointData[14].x(), pointData[14].y(), pointData[14].z());

		glVertex3f(pointData[16].x(), pointData[16].y(), pointData[16].z() + 4);
		glVertex3f(pointData[14].x(), pointData[14].y(), pointData[14].z() + 4);

		glVertex3f(pointData[16].x(), pointData[16].y(), pointData[16].z() + 8);
		glVertex3f(pointData[14].x(), pointData[14].y(), pointData[14].z() + 8);

		glVertex3f(pointData[16].x(), pointData[16].y(), pointData[16].z() + 12);
		glVertex3f(pointData[14].x(), pointData[14].y(), pointData[14].z() + 12);

		glVertex3f(pointData[16].x(), pointData[16].y(), pointData[16].z());
		glVertex3f(pointData[15].x(), pointData[15].y(), pointData[15].z());

		glVertex3f(pointData[16].x(), pointData[16].y(), pointData[16].z() + 4);
		glVertex3f(pointData[15].x(), pointData[15].y(), pointData[15].z() + 4);

		glVertex3f(pointData[16].x(), pointData[16].y(), pointData[16].z() + 8);
		glVertex3f(pointData[15].x(), pointData[15].y(), pointData[15].z() + 8);

		glVertex3f(pointData[16].x(), pointData[16].y(), pointData[16].z() + 12);
		glVertex3f(pointData[15].x(), pointData[15].y(), pointData[15].z() + 12);
		glEnd();
		#pragma endregion
	}
	
	#pragma region 根據開始和最後的點來畫
	for (int i = 0; i < startVector.size(); i++)
	{
		QVector3D startPoint = startVector[i];
		QVector3D endPoint = endVector[i];

		int dx = (startPoint.x() < endPoint.x()) ? 1 : -1;
		int dy = (startPoint.y() < endPoint.y()) ? 1 : -1;
		int dz = (startPoint.z() < endPoint.z()) ? 1 : -1;

		if (name == "roof/cross_gable")
		{
			float currentZ = startPoint.z();
			while (currentZ * dz < endPoint.z() * dz)
			{
				float currentX = startPoint.x();
				float nextZ = GetNextValue(currentZ, endPoint.z(), dz);

				while (currentX * dx < endPoint.x() * dx)
				{
					float nextX = GetNextValue(currentX, endPoint.x(), dx);

					float Prograss = (currentZ - startPoint.z()) / (endPoint.z() - startPoint.z());
					float NextPrograss = (nextZ - startPoint.z()) / (endPoint.z() - startPoint.z());

					float currentY = (endPoint.y() - startPoint.y()) * Prograss + startPoint.y();
					float nextY = (endPoint.y() - startPoint.y()) * NextPrograss + startPoint.y();

					// 畫線
					glColor3f(0, 0, 0);
					glBegin(GL_LINES);
					glVertex3f(currentX, currentY, currentZ);
					glVertex3f(nextX, currentY, currentZ);

					glVertex3f(currentX, currentY, currentZ);
					glVertex3f(currentX, nextY, nextZ);
					glEnd();

					currentX = nextX;
				}
				currentZ = nextZ;
			}
		}
		else if (name == "roof/Triangle")
		{
			float currentZ = startPoint.z();
			while (currentZ * dz <= endPoint.z() * dz)
			{
				float nextZ = GetNextValue(currentZ, endPoint.z(), dz);

				float Prograss = (currentZ - startPoint.z()) / (endPoint.z() - startPoint.z());
				float NextPrograss = (nextZ - startPoint.z()) / (endPoint.z() - startPoint.z());

				float currentY = (endPoint.y() - startPoint.y()) * Prograss + startPoint.y();
				float nextY = (endPoint.y() - startPoint.y()) * NextPrograss + startPoint.y();

				// 畫線
				glColor3f(0, 0, 0);
				glBegin(GL_LINES);
				glVertex3f(startPoint.x(), startPoint.y(), currentZ);
				glVertex3f(startPoint.x(), currentY, currentZ);
				glEnd();

				currentZ = nextZ;
			}
		}
		else if (startPoint.y() == endPoint.y())
		{
			float currentZ = startPoint.z();
			while (currentZ * dz < endPoint.z() * dz)
			{
				float currentX = startPoint.x();
				float nextZ = GetNextValue(currentZ, endPoint.z(), dz);

				while (currentX * dx < endPoint.x() * dx)
				{
					float nextX = GetNextValue(currentX, endPoint.x(), dx);

					// 畫線
					glColor3f(0, 0, 0);
					glBegin(GL_LINES);
					glVertex3f(currentX, startPoint.y(), currentZ);
					glVertex3f(nextX, startPoint.y(), currentZ);

					glVertex3f(currentX, startPoint.y(), currentZ);
					glVertex3f(currentX, startPoint.y(), nextZ);
					glEnd();

					currentX = nextX;
				}
				currentZ = nextZ;
			}
		}
		else if (startPoint.z() == endPoint.z())
		{
			float currentY = startPoint.y();
			while (currentY * dy < endPoint.y() * dy)
			{
				float nextY = GetNextValue(currentY, endPoint.y(), dy);

				float currentX = startPoint.x();
				while (currentX * dx < endPoint.x() * dx)
				{
					float nextX = GetNextValue(currentX, endPoint.x(), dx);

					// 畫線
					glColor3f(0, 0, 0);
					glBegin(GL_LINES);
					glVertex3f(currentX, currentY, startPoint.z());
					glVertex3f(currentX, nextY, startPoint.z());

					glVertex3f(currentX, currentY, startPoint.z());
					glVertex3f(nextX, currentY, startPoint.z());
					glEnd();

					currentX = nextX;
				}
				currentY = nextY;
			}
		}
		else if (startPoint.x() == endPoint.x())
		{
			float currentY = startPoint.y();
			while (currentY * dy < endPoint.y() * dy)
			{
				float currentZ = startPoint.z();
				float nextY = GetNextValue(currentY, endPoint.y(), dy);

				while (currentZ * dz < endPoint.z() * dz)
				{
					float nextZ = GetNextValue(currentZ, endPoint.z(), dz);

					// 畫線
					glColor3f(0, 0, 0);
					glBegin(GL_LINES);
					glVertex3f(startPoint.x(), currentY, currentZ);
					glVertex3f(startPoint.x(), currentY, nextZ);

					glVertex3f(startPoint.x(), currentY, currentZ);
					glVertex3f(startPoint.x(), nextY, currentZ);
					glEnd();

					currentZ = nextZ;
				}
				currentY = nextY;
			}
		}
	}
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
	else if (info->name == "wall/multi_window")
	{
		NormalParams nParams = info->nParams;
		MultiWindowParams windowParams = info->multiWindowParams;

		float rAW = qBound(-0.5, windowParams.windowA.RatioWidth - 0.5, 0.5);
		float rAH = qBound(-0.5, windowParams.windowA.RatioHeight - 0.5, 0.5);

		float rBW = qBound(-0.5, windowParams.windowB.RatioWidth - 0.5, 0.5);
		float rBH = qBound(-0.5, windowParams.windowB.RatioHeight - 0.5, 0.5);
		
		QVector3D centerAPos = nParams.TranslatePoint + QVector3D(-nParams.XLength * rAW / 0.5, nParams.YLength * rAH, 0);
		QVector3D centerBPos = nParams.TranslatePoint + QVector3D(-nParams.XLength * rBW / 0.5, nParams.YLength * rBH, 0);

		//////////////////////////////////////////////////////////////////////////
		// -z 看過去
		//////////////////////////////////////////////////////////////////////////
		// 最外側
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		
		outputPoint.push_back(QVector3D(windowParams.windowA.WindowWidth, -windowParams.windowA.WindowHeight, -nParams.ZLength) + centerAPos);
		outputPoint.push_back(QVector3D(-windowParams.windowA.WindowWidth, -windowParams.windowA.WindowHeight, -nParams.ZLength) + centerAPos);
		outputPoint.push_back(QVector3D(-windowParams.windowA.WindowWidth, windowParams.windowA.WindowHeight, -nParams.ZLength) + centerAPos);
		outputPoint.push_back(QVector3D(windowParams.windowA.WindowWidth, windowParams.windowA.WindowHeight, -nParams.ZLength) + centerAPos);

		outputPoint.push_back(QVector3D(windowParams.windowB.WindowWidth, -windowParams.windowB.WindowHeight, -nParams.ZLength) + centerBPos);
		outputPoint.push_back(QVector3D(-windowParams.windowB.WindowWidth, -windowParams.windowB.WindowHeight, -nParams.ZLength) + centerBPos);
		outputPoint.push_back(QVector3D(-windowParams.windowB.WindowWidth, windowParams.windowB.WindowHeight, -nParams.ZLength) + centerBPos);
		outputPoint.push_back(QVector3D(windowParams.windowB.WindowWidth, windowParams.windowB.WindowHeight, -nParams.ZLength) + centerBPos);

		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(windowParams.windowA.WindowWidth, -windowParams.windowA.WindowHeight, nParams.ZLength) + centerAPos);
		outputPoint.push_back(QVector3D(-windowParams.windowA.WindowWidth, -windowParams.windowA.WindowHeight, nParams.ZLength) + centerAPos);
		outputPoint.push_back(QVector3D(-windowParams.windowA.WindowWidth, windowParams.windowA.WindowHeight, nParams.ZLength) + centerAPos);
		outputPoint.push_back(QVector3D(windowParams.windowA.WindowWidth, windowParams.windowA.WindowHeight, nParams.ZLength) + centerAPos);

		outputPoint.push_back(QVector3D(windowParams.windowB.WindowWidth, -windowParams.windowB.WindowHeight, nParams.ZLength) + centerBPos);
		outputPoint.push_back(QVector3D(-windowParams.windowB.WindowWidth, -windowParams.windowB.WindowHeight, nParams.ZLength) + centerBPos);
		outputPoint.push_back(QVector3D(-windowParams.windowB.WindowWidth, windowParams.windowB.WindowHeight, nParams.ZLength) + centerBPos);
		outputPoint.push_back(QVector3D(windowParams.windowB.WindowWidth, windowParams.windowB.WindowHeight, nParams.ZLength) + centerBPos);

		// 要先判斷兩個點是差距多少度
		//QVector3D midAPoint = QVector3D()
	}
	else if (info->name == "roof/Triangle")
	{
		NormalParams nParams = info->nParams;
		TriangleParams triParams = info->triangleParams;

		float r = qBound(-0.5, triParams.ratioX - 0.5, 0.5) * 2;
		float Moveable = qMax(nParams.XLength - 1, 0);

		//////////////////////////////////////////////////////////////////////////
		// 從 x 的地方看過去
		//////////////////////////////////////////////////////////////////////////

		outputPoint.push_back(QVector3D(-nParams.XLength, 0, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, 0, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, r * Moveable) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(nParams.XLength, 0, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, 0, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, r * Moveable) + nParams.TranslatePoint);
	}
	else if (info->name == "roof/cross_gable")
	{
		//////////////////////////////////////////////////////////////////////////
		// 從 x 的地方看過去
		//////////////////////////////////////////////////////////////////////////

		NormalParams nParams = info->nParams;
		CrossGableParams gableParams = info->gableParams;
		outputPoint.push_back(QVector3D(-nParams.XLength, 0, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, 0) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, 0, nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(-nParams.XLength, 0, -nParams.ZLength - gableParams.ZOffset) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength + gableParams.YOffset, 0) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, 0, nParams.ZLength + gableParams.ZOffset) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(nParams.XLength, 0, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, 0) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, 0, nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(nParams.XLength, 0, -nParams.ZLength - gableParams.ZOffset) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength + gableParams.YOffset, 0) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, 0, nParams.ZLength + gableParams.ZOffset) + nParams.TranslatePoint);

		// 凸出來的部分
		float shapeRatio = (float)nParams.ZLength / nParams.YLength;
		float shapeZLength = nParams.ZLength - shapeRatio * gableParams.ConvexHeight + gableParams.ZOffset;
		float r = qBound(-0.5, gableParams.ratio - 0.5, 0.5) * 2;
		float Moveable = qMax(nParams.XLength - 1, 0);

		outputPoint.push_back(QVector3D(r * Moveable, gableParams.ConvexHeight, -shapeZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(r * Moveable, nParams.YLength + gableParams.YOffset, 0) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(r * Moveable + gableParams.ConvexWidth, 0, -nParams.ZLength - gableParams.ZOffset) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(r * Moveable - gableParams.ConvexWidth, 0, -nParams.ZLength -gableParams.ZOffset) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(r * Moveable, gableParams.ConvexHeight, -nParams.ZLength - gableParams.ZOffset) + nParams.TranslatePoint);
	}
	return outputPoint;
}
float OpenGLWidget::GetNextValue(float currentValue, float max, int dir)
{
	float nextValue = currentValue + 4 * dir;					// 下一個值

	// 超出的形況判斷
	if (nextValue * dir >= max * dir && currentValue * dir < max * dir)
		nextValue = max;

	// 判斷接下來的值，有沒有小於卡榫的洞的值，如果有就將 NextValue 切兩半
	if (abs(max - nextValue) > 0 && abs(max - nextValue) < 2)
		nextValue = (currentValue + max) / 2;
	return nextValue;
}
