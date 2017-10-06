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

	// �u�e Front Face
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

void OpenGLWidget::AddComboBoxList()
{
	for (int i = 0; i < queueInfo.size(); i++)
		comboBox->addItem(queueInfo[i]->name);
}

void OpenGLWidget::UpdateByParams(int index, int width, int height, float ratioAW, float ratioAH, float ratioBW, float ratioBH)
{
	#pragma region �ھ� Case �� Params
	switch (index)
	{
	case 0:
		{
		// �̤j�i�H�Y��d����(�]�N�O��)
		int MinWidth = -1;
		int MinHeight = -1;

		#pragma region ���a�O & ���᪺��T�A�P�_�̤p���϶��b����
		for (int i = 0; i < queueInfo.size(); i++)
		{
			// ���᪺�ܡA�O�n�� ZLength
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
	}
	#pragma endregion
	#pragma region �Ĥ@�ӳW�h => Parent ���O�� Child �j
	QVector<NodeInfo *> infoArray;
	infoArray.push_back(info->Root);

	while (infoArray.size() > 0)
	{
		NodeInfo *currentInfo = infoArray[0];
		
		// �n�P�_�O���O�b�d��
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
				childInfo->nParams.ZLength = qBound(0, childInfo->nParams.ZLength, currentInfo->nParams.ZLength -1);		// �]���������p�׬� 1�A�ҥH�᭱�~�n��1
				
				// ���]�p�שT�w�� 0
				childPos.setZ(qBound(pos.z() - currentInfo->nParams.ZLength + childInfo->nParams.ZLength, childPos.z(), pos.z() + currentInfo->nParams.ZLength - childInfo->nParams.ZLength));
			}
			else if (childInfo->name == "roof/Triangle")
			{
				// �T���θ�W���� Case �ܹ��A�ߤ@���@�˪��a��b��A�L�i�H�h�@�ӫ��𪺼e��
				childPos.setX(qBound(pos.x() - currentInfo->nParams.XLength + childInfo->nParams.XLength, childPos.x(), pos.x() + currentInfo->nParams.XLength - childInfo->nParams.XLength));
				childInfo->nParams.ZLength = qBound(1, childInfo->nParams.ZLength, currentInfo->nParams.ZLength + 1);		// �T���Φ]���O�b �� �M �����U�A�ҥH�S������ 1
				
				// �]�������𪺫p�סA�ҥH�n�첾��ӳ��(�p�װߤ@�A���O���~����ӳ��)
				childPos.setZ(qBound(pos.z() - currentInfo->nParams.ZLength + childInfo->nParams.ZLength - 2, childPos.z(), pos.z() + currentInfo->nParams.ZLength - childInfo->nParams.ZLength - 2));
			}

			// �[�W Child ���A�⥦�������}�C��
			infoArray.push_back(currentInfo->childNode[i]);
		}

		// ��̫e���� POP ��
		infoArray.pop_front();
	}
	#pragma endregion
	#pragma region �ĤG�ӳW�h => �h������d��
	if (queueInfo[index]->name == "wall/multi_window")
	{
		NodeInfo* MultiWindowInfo = queueInfo[index];
		float GapLength = MultiWindowInfo->multiWindowParams.windowA.WindowWidth + MultiWindowInfo->multiWindowParams.windowB.WindowWidth + 1;
		float GapRatio = GapLength / MultiWindowInfo->nParams.XLength / 2;

		float RatioBorderA = (float)(MultiWindowInfo->multiWindowParams.windowA.WindowWidth + 1) / MultiWindowInfo->nParams.XLength / 2;
		float RatioBorderB = (float)(MultiWindowInfo->multiWindowParams.windowB.WindowWidth + 1) / MultiWindowInfo->nParams.XLength / 2;

		// 1. �P�_�O�_�����
		// 2. �P�_�O�_���쥪��
		// 3. �P�_�O�_����k��
		if (qAbs(MultiWindowInfo->multiWindowParams.windowA.RatioWidth - MultiWindowInfo->multiWindowParams.windowB.RatioWidth) < GapRatio ||
			(MultiWindowInfo->multiWindowParams.windowA.RatioWidth - RatioBorderA <= 0 || MultiWindowInfo->multiWindowParams.windowB.RatioWidth - RatioBorderB <= 0) ||
			(MultiWindowInfo->multiWindowParams.windowA.RatioWidth + RatioBorderA >= 1 || MultiWindowInfo->multiWindowParams.windowB.RatioWidth + RatioBorderB >= 1))
		{
			// �令�H�e�� Ratio
			MultiWindowInfo->multiWindowParams.windowA.RatioWidth = LastRatioA;
			MultiWindowInfo->multiWindowParams.windowB.RatioWidth = LastRatioB;
		}
		else
		{
			LastRatioA = MultiWindowInfo->multiWindowParams.windowA.RatioWidth;
			LastRatioB = MultiWindowInfo->multiWindowParams.windowB.RatioWidth;
		}
	}
	#pragma endregion

}

void OpenGLWidget::InitModelParams()
{
	#pragma region �a�O
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
	#pragma region �@�ӵ���
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
	#pragma region ��
	NodeInfo *door_entry = new NodeInfo;
	door_entry->nParams.XLength = 1;
	door_entry->nParams.YLength = 10;
	door_entry->nParams.ZLength = 15;
	door_entry->nParams.TranslatePoint = QVector3D(23, 11, 1);
	
	door_entry->doorParams.ratio = 0.5;
	door_entry->doorParams.DoorWidth = 5;
	door_entry->doorParams.DoorHeight = 9;							// �q���U���W��Ӱ��� (�ҥH���a�� DoorHeight x 2)
	door_entry->name = "wall/door_entry";
	ground->childNode.push_back(door_entry);

	queueInfo.push_back(door_entry);
	#pragma endregion
	#pragma region ��ӵ�
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
	#pragma region �T����
	NodeInfo *roof_leftTriangle = new NodeInfo;
	roof_leftTriangle->nParams.XLength = 1;
	roof_leftTriangle->nParams.YLength = 10;								// �q�����WYLength
	roof_leftTriangle->nParams.ZLength = 16;
	roof_leftTriangle->nParams.TranslatePoint = QVector3D(-23, 21, 0);
	roof_leftTriangle->name = "roof/Triangle";

	roof_leftTriangle->triangleParams.ratioX = 0.5;
	single_window->childNode.push_back(roof_leftTriangle);

	queueInfo.push_back(roof_leftTriangle);

	roof_leftTriangle = new NodeInfo;
	roof_leftTriangle->nParams.XLength = 1;
	roof_leftTriangle->nParams.YLength = 10;								// �q�����WYLength
	roof_leftTriangle->nParams.ZLength = 16;
	roof_leftTriangle->nParams.TranslatePoint = QVector3D(23, 21, 0);
	roof_leftTriangle->name = "roof/Triangle";

	roof_leftTriangle->triangleParams.ratioX = 0.5;
	door_entry->childNode.push_back(roof_leftTriangle);

	queueInfo.push_back(roof_leftTriangle);
	#pragma endregion
	#pragma region �γ�
	NodeInfo *gable = new NodeInfo;
	gable->nParams.XLength = 24;
	gable->nParams.YLength = 10;
	gable->nParams.ZLength = 16;
	gable->nParams.TranslatePoint = QVector3D(0, 21, 0);
	gable->name = "roof/cross_gable";

	gable->gableParams.YOffset = 0.625;
	gable->gableParams.ZOffset = 1;

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
	#pragma region BFS �j�M��ʾ� & �e�X��ɩФl
	QVector<NodeInfo *> TreeTraverse;
	TreeTraverse.push_back(info->Root);
	while (TreeTraverse.size() > 0)
	{
		NodeInfo *currentNode = TreeTraverse[0];

		// �N�����ন 3D �I�A�ù��X��
		QVector<QVector3D> ObjectPoint = TransformParamToModel(currentNode);
		DrawModelByName(currentNode->name, ObjectPoint);

		// ������� child ��i��
		for (int i = 0; i < currentNode->childNode.size(); i++)
			TreeTraverse.push_back(currentNode->childNode[i]);

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
		#pragma region ��
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
		#pragma endregion
		#pragma region �u
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
		#pragma region ��
		#pragma region �e
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
		#pragma region ��
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
		#pragma region �W
		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(10);
		tempArray.push_back(11);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region �U
		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(9);
		tempArray.push_back(8);
		tempArray.push_back(0);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region ��
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		tempArray.push_back(11);
		tempArray.push_back(8);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region �k
		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(9);
		tempArray.push_back(10);
		tempArray.push_back(2);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region ����
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
		#pragma region �u
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
		
		// ���
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

		// �e��M���
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
		#pragma region ��
		#pragma region �e
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
		#pragma region ��
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
		#pragma region �W
		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(2);
		tempArray.push_back(10);
		tempArray.push_back(11);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region �U
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
		#pragma region ��
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(3);
		tempArray.push_back(11);
		tempArray.push_back(8);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region �k
		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(9);
		tempArray.push_back(10);
		tempArray.push_back(2);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region ����
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
		#pragma region �u
		// �e��
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

		// ���
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

		// �e��M���
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
		#pragma region ��
		#pragma region �e
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
		#pragma region ��
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
		#pragma region ��
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
		#pragma region �u
		#pragma region �e
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
		#pragma region ��
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
		#pragma region ��
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
		#pragma region ��
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
		#pragma region �u
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

		// ����
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
		#pragma region ��
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(1);
		tempArray.push_back(4);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(1);
		tempArray.push_back(2);
		tempArray.push_back(5);
		tempArray.push_back(4);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(9);
		tempArray.push_back(10);
		tempArray.push_back(7);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(10);
		tempArray.push_back(11);
		tempArray.push_back(8);
		tempArray.push_back(7);
		FaceIndex.push_back(tempArray);

		// ����
		tempArray.push_back(4);
		tempArray.push_back(10);
		tempArray.push_back(9);
		tempArray.push_back(3);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(4);
		tempArray.push_back(5);
		tempArray.push_back(11);
		tempArray.push_back(10);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(0);
		tempArray.push_back(3);
		tempArray.push_back(9);
		tempArray.push_back(6);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(5);
		tempArray.push_back(2);
		tempArray.push_back(8);
		tempArray.push_back(11);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(1);
		tempArray.push_back(0);
		tempArray.push_back(6);
		tempArray.push_back(7);
		FaceIndex.push_back(tempArray);

		tempArray.push_back(2);
		tempArray.push_back(1);
		tempArray.push_back(7);
		tempArray.push_back(8);
		FaceIndex.push_back(tempArray);
		#pragma endregion
		#pragma region �u
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

		// ����
		tempArray.clear();
		tempArray.push_back(0);
		tempArray.push_back(6);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(1);
		tempArray.push_back(7);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(2);
		tempArray.push_back(8);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(3);
		tempArray.push_back(9);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(4);
		tempArray.push_back(10);
		LineIndex.push_back(tempArray);

		tempArray.clear();
		tempArray.push_back(5);
		tempArray.push_back(11);
		LineIndex.push_back(tempArray);
		#pragma endregion

	}

	#pragma region �ھ� FaceIndex �h�e�I
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
	#pragma region �ھ� LineIndex �h�u
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
		// �q -x ���a��ݹL�h
		//////////////////////////////////////////////////////////////////////////

		float rz = qBound(-0.5f, singleWindowParams.RatioWidth - 0.5f, 0.5f);
		float ry = qBound(-0.5f, singleWindowParams.RatioHeight - 0.5f, 0.5f);

		float MoveableZ = qMax(nParams.ZLength - singleWindowParams.WindowWidth - 1, 0);
		float MoveableY = qMax(nParams.YLength - singleWindowParams.WindowHeight - 1, 0);

		// �̥~��
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(-nParams.XLength, -singleWindowParams.WindowWidth + MoveableY * ry, -singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, -singleWindowParams.WindowWidth + MoveableY * ry, singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, singleWindowParams.WindowWidth + MoveableY * ry, singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(-nParams.XLength, singleWindowParams.WindowWidth + MoveableY * ry, -singleWindowParams.WindowHeight + MoveableZ * rz) + nParams.TranslatePoint);

		// ����
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
		// �q x ���a��ݹL�h
		//////////////////////////////////////////////////////////////////////////

		float r = qBound(-0.5f, doorParams.ratio - 0.5f, 0.5f);

		float Moveable = qMax(nParams.ZLength - doorParams.DoorWidth - 1, 1);

		// �̥~��
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, -nParams.ZLength) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, nParams.YLength, nParams.ZLength) + nParams.TranslatePoint);

		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength, -doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength + doorParams.DoorHeight * 2, -doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);
		outputPoint.push_back(QVector3D(nParams.XLength, -nParams.YLength + doorParams.DoorHeight * 2, doorParams.DoorWidth - Moveable * r) + nParams.TranslatePoint);

		// ����
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
		// -z �ݹL�h
		//////////////////////////////////////////////////////////////////////////
		// �̥~��
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

		// �n���P�_����I�O�t�Z�h�֫�
		//QVector3D midAPoint = QVector3D()
	}
	else if (info->name == "roof/Triangle")
	{
		NormalParams nParams = info->nParams;
		TriangleParams triParams = info->triangleParams;

		float r = qBound(-0.5, triParams.ratioX - 0.5, 0.5);
		float Moveable = qMax(nParams.XLength - 1, 0);

		//////////////////////////////////////////////////////////////////////////
		// �q x ���a��ݹL�h
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
		// �q x ���a��ݹL�h
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
	}
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
