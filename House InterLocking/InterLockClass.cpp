#include "InterLockClass.h"

InterLockClass::InterLockClass(char *inputFile)
{
	#pragma region Ū���Y
	cout << "========== Ū���ɮ� ==========" << endl;
	QString tempStr = QString(inputFile);
	QFile file(tempStr);
	if (!file.open(QIODevice::ReadOnly))
	{
		cout << "�L�k���}�ɮ� !!" << endl;
		return;
	}
	QTextStream ss(&file);
	int tempInt, totalNumber;
	float tempFloat, tempX, tempY, tempZ;
	int tempType;

	QString InputFileStr(inputFile);
	InputFileStr = InputFileStr.replace("\\", "/");
	QStringList FilePathList = InputFileStr.split("/");
	FilePathLocation = "";
	for (int i = 0; i < FilePathList.length() - 1; i++)
		FilePathLocation += FilePathList[i] + "/";

	ss >> totalNumber;
	cout << "�`�@�Ӽ� => " << totalNumber << endl;
	#pragma endregion
	#pragma region Ū�̭����
	for (int i = 0; i < totalNumber; i++)
	{
		MyMesh *tempMesh = new MyMesh;
		ModelInfo *info = new ModelInfo();
		
		// Ū�ɮצW��
		ss >> tempStr;
		OpenMesh::IO::read_mesh(*tempMesh, (FilePathLocation + tempStr + ".obj").toStdString().data());
		tempStr.replace("model_part", "");
		info->PartNumber = tempStr.toInt();
		cout << "Part �� => " << info->PartNumber << endl;
		cout << "�I�� => " << tempMesh->n_vertices() << endl << endl;

		tempMesh->request_vertex_status();
		tempMesh->request_edge_status();
		tempMesh->request_face_status();
		ModelsArray.push_back(tempMesh);

		ss >> info->PartName;
		ss >> tempInt;
		
		// �̭����d�g����T�A�Ȯɥ����|�Ψ�
		for (int j = 0; j < tempInt / 2; j++)
			for (int k = 0; k < 6; k++)
			{
				ss >> tempX >> tempY >> tempZ >> tempType;
				//qDebug() << tempX << tempY << tempZ << tempType;
				/*switch (k)
				{
				case 0:
				case 3:
					info->InterLockType.push_back(tempType);
					info->InterLockingFace.push_back(MyMesh::Point(tempX, tempY, tempZ));
					break;
				default:
					info->InterLockingFace.push_back(MyMesh::Point(tempX, tempY, tempZ));
					break;
				}*/
			}

		InfoArray.push_back(info);
	}
	file.close();
	cout << "========== Ū������ ==========" << endl;
	#pragma endregion
}
InterLockClass::~InterLockClass()
{
	// �M�ŰO����
	for (int i = 0; i < ModelsArray.length(); i++)
		delete ModelsArray[i];
	for (int i = 0; i < InfoArray.length(); i++)
		delete InfoArray[i];
}

void InterLockClass::SplitInSmallSize()
{
	cout << "========== �}�l�����p���� ==========" << endl;
	int SplitCount = 0;
	int lastSplitCount = 0;
	for (int i = 0; i < InfoArray.length(); i++)
	{
		QVector<MyMesh::Point> PointArray;
		QVector<MyMesh::VertexHandle> vhandle;								// Vertex Handle

		// �N�Ĥ@�Ӧr�ܤj�g
		InfoArray[i]->PartName = InfoArray[i]->PartName[0].toUpper() + InfoArray[i]->PartName.mid(1, InfoArray[i]->PartName.length() - 1);

		#pragma region �γ�����
		// Gable ���γ�
		if (InfoArray[i]->PartName.endsWith("/gable"))
		{
			#pragma region 	�ư����~���p
			if (ModelsArray[i]->n_vertices() != 20)
			{
				cout << "Root Gable �����~" << endl;
				return;
			}
			#pragma endregion
			#pragma region �[�|���I x 2
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			int offsetArray[] = { 0,1,2,3,16,19 ,18, 17 };
			for (int j = 0; j < 8; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region �}�l�n�[����
			QVector<MyMesh::VertexHandle>	vhandle;						// Vertex Handle
			
			// �s�W info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;

			// 0 �������A��n�����̶]
			float dx = (PointArray[7][0] - PointArray[4][0] > 0) ? 1 : -1;
			float dz = (PointArray[5][2] - PointArray[4][2] > 0) ? 1 : -1;

			// �q�o�Ӷ}�l
			float CurrentX;
			float CurrentZ;

			// �U�@�Ӫ� X, Z
			float NextX;
			float NextZ;

			// �}�l����
			float StartX = PointArray[4][0];
			float StartZ = PointArray[4][2];

			// ��������
			float EndX = PointArray[7][0];
			float EndZ = PointArray[5][2];

			// �γ��� offset
			float offsetY = PointArray[0][1] - PointArray[4][1];

			CurrentX = StartX;
			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);

				CurrentX = StartX;
				while (CurrentX * dx < EndX * dx)
				{
					NextX = GetNextValue(CurrentX, dx, EndX);

					float Prograss = (CurrentZ - StartZ) / (EndZ - StartZ);
					float NextPrograss = (NextZ - StartZ) / (EndZ - StartZ);

					float CurrentY = (PointArray[5][1] - PointArray[4][1]) * Prograss + PointArray[4][1];
					float NextY = (PointArray[5][1] - PointArray[4][1]) * NextPrograss + PointArray[4][1];
					
					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();
					if (dz < 0) 
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �᭱
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
						else 
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentX = NextX;
				}

				CurrentZ = NextZ;
			}
			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;
			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
		}
		
		// Cross Gable ���γ�
		else if (InfoArray[i]->PartName.endsWith("/cross_gable"))
		{
			// �@�Ǹ�T
			QVector<MyMesh::Point> LDPoint;				// Left Down Point
			QVector<MyMesh::Point> RDPoint;				// Right Down Point

			#pragma region �L�T���γB(���)
			#pragma region �[�|���I x 2
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			QVector<int> offsetArray = { 25,26,34,33,24,27,35,32 };
			for (int j = 0; j < 8; j++)
			{

				MyMesh::Point tempP;
				tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region �}�l�n�[����
			QVector<MyMesh::VertexHandle>	vhandle;						// Vertex Handle
			
			// �s�W info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;

			// 0 �������A��n�����̶]
			float dx = (PointArray[7][0] - PointArray[4][0] > 0) ? 1 : -1;
			float dz = (PointArray[5][2] - PointArray[4][2] > 0) ? 1 : -1;

			// �q�o�Ӷ}�l
			float CurrentX;
			float CurrentZ;

			// �U�@�Ӫ� X, Z
			float NextX;
			float NextZ;

			// �}�l����
			float StartX = PointArray[4][0];
			float StartZ = PointArray[4][2];

			// ��������
			float EndX = PointArray[7][0];
			float EndZ = PointArray[5][2];

			// �γ��� offset
			float offsetY = PointArray[0][1] - PointArray[4][1];

			CurrentX = StartX;
			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);

				CurrentX = StartX;
				while (CurrentX * dx < EndX * dx)
				{
					NextX = GetNextValue(CurrentX, dx, EndX);

					float Prograss = (CurrentZ - StartZ) / (EndZ - StartZ);
					float NextPrograss = (NextZ - StartZ) / (EndZ - StartZ);

					float CurrentY = (PointArray[5][1] - PointArray[4][1]) * Prograss + PointArray[4][1];
					float NextY = (PointArray[5][1] - PointArray[4][1]) * NextPrograss + PointArray[4][1];
					
					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();
					if (dz < 0) 
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �᭱
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
						}
						else 
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentX = NextX;
				}

				CurrentZ = NextZ;
			}
			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;
			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
			#pragma endregion
			#pragma region ���T���γB(�e��)
			#pragma region �[�K���I (�W�b��)
			MyMesh::VertexIter v1_it = ModelsArray[i + 1]->vertices_begin();
			MyMesh::VertexIter v2_it = ModelsArray[i + 2]->vertices_begin();

			offsetArray.clear();
			offsetArray = { 33, 34, -5, -20, -2, 31, 25 };

			PointArray.clear();
			for (int j = 0; j < 7; j++)
			{
				MyMesh::Point tempP;
				if (offsetArray[j] == -20)
					tempP = ModelsArray[i + 1]->point(v1_it + abs(offsetArray[j]));
				else if (offsetArray[j] < 0)
					tempP = ModelsArray[i + 2]->point(v2_it + abs(offsetArray[j]));
				else
				{
					tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
					if (abs(tempP[2]) < 0.01f)
						tempP[2] = 0;
				}
				PointArray.push_back(tempP);
			}
			PointArray.push_back((PointArray[0] + PointArray[6]) / 2);
			#pragma endregion
			#pragma region �[���� info & �@�Ǹ�T(�k��)
			// �[�W���� & �̧C�I����T
			float Height = ModelsArray[i]->point(v_it + 25)[1] - ModelsArray[i]->point(v_it + 24)[1];
			float MinHeight = ModelsArray[i]->point(v_it + 31)[1];
			float AppendZ = ModelsArray[i]->point(v_it + 31)[2] - ModelsArray[i]->point(v_it + 30)[2];

			// �s�W info
			splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i + 1;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i + 1]->PartNumber;
			splitInfo->PartName = InfoArray[i + 1]->PartName;

			QVector<MyMesh::Point> TopVertexArray;
			QVector<MyMesh::Point> BotVertexArray;
			QVector<float> SideZArray;

			// ����X Top �I�� Array
			dx = (PointArray[7][0] - PointArray[0][0] > 0) ? 1 : -1;
			dz = (PointArray[1][2] - PointArray[0][2] > 0) ? 1 : -1;

			// ����X Bottom �I�� Array
			float MidX = PointArray[7][0];
			PointArray[3][0] = MidX;		// �ץ��~�t�A�쥻�� X �b��bug

			CurrentX = PointArray[1][0];
			EndX = PointArray[3][0];
			bool GoSlash = false;

			// X �b
			MyMesh::Point tempPoint;
			while (CurrentX * dx < EndX * dx)
			{
				NextX = GetNextValue(CurrentX, dx, EndX);

				if (CurrentX * dx >= MidX * dx)
				{
					float prograss = (CurrentX - PointArray[3][0]) / (PointArray[2][0] - PointArray[3][0]);
					tempPoint = (PointArray[2] - PointArray[3]) * prograss + PointArray[3];

					// Bot ���I�A�p�G�@�}�l���O 0 ���ܤ~�n�[
					if (prograss != 0 && prograss != 1 && !GoSlash)
					{
						TopVertexArray.push_back(MyMesh::Point(PointArray[2][0], PointArray[0][1], PointArray[0][2]));
						BotVertexArray.push_back(PointArray[2]);
					}
					GoSlash = true;

					TopVertexArray.push_back(MyMesh::Point(CurrentX, PointArray[0][1], PointArray[0][2]));
					BotVertexArray.push_back(tempPoint);
				}
				else
				{
					TopVertexArray.push_back(MyMesh::Point(CurrentX, PointArray[0][1], PointArray[0][2]));
					BotVertexArray.push_back(MyMesh::Point(CurrentX, PointArray[1][1], PointArray[1][2]));
				}

				CurrentX = NextX;
			}
			// �P�_�O�_���A�̩w�~�t�H���A�p�G�S���A�N��n���I�[�i��
			if (abs((TopVertexArray[TopVertexArray.length() - 1][0] - NextX)) > 0.01f)
			{
				TopVertexArray.push_back(PointArray[7]);
				BotVertexArray.push_back(PointArray[3]);
			}

			// Z �b
			float MidZ = PointArray[3][2];
			CurrentZ = (PointArray[0][2] < 0.01f) ? 0 : PointArray[0][2];
			EndZ = PointArray[1][2];
			GoSlash = false;
			
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);

				if (CurrentZ * dz >= MidZ * dz)
				{
					float prograss = (CurrentZ - PointArray[3][2]) / (PointArray[2][2] - PointArray[3][2]);
					tempPoint = (PointArray[2] - PointArray[3]) * prograss + PointArray[3];

					// Bot ���I�A�p�G�@�}�l���O 0 ���ܤ~�n�[
					if (prograss != 0 && !GoSlash)
						SideZArray.push_back(PointArray[3][2]);
					GoSlash = true;

					SideZArray.push_back(tempPoint[2]);
				}
				else
					SideZArray.push_back(CurrentZ);

				CurrentZ = NextZ;
			}
			// �P�_�O�_���A�̩w�~�t�H���A�p�G�S���A�N��n���I�[�i��
			if (abs((SideZArray[SideZArray.length() - 1] - NextZ)) > 0.01f)
				SideZArray.push_back(PointArray[2][2]);
			#pragma endregion
			#pragma region �[�F��i�}�C��(�k��)
			MyMesh::Point lastTopLeftPoint(0,0,0);	// For �W��A�n�^���I���ɭԦa���s�Ϊ�	

			for (CurrentZ = 0; CurrentZ < SideZArray.length() - 1; CurrentZ++)
			{
				float prograss = (SideZArray[CurrentZ] - BotVertexArray[0][2]) / (TopVertexArray[0][2] - BotVertexArray[0][2]);
				float CurrentY = prograss * (TopVertexArray[0][1] - BotVertexArray[0][1]) + BotVertexArray[0][1];

				// �Ȧ���Ϊ��s�b
				float tempPrograss = (SideZArray[CurrentZ + 1] - PointArray[3][2]) / (PointArray[2][2] - PointArray[3][2]);
				tempPoint = (PointArray[2] - PointArray[3]) * tempPrograss + PointArray[3];

				MyMesh::Point lastLeftPoint(0, 0, 0);
				QVector<MyMesh::Point> EndPointVector;
				for (CurrentX = 0; CurrentX < TopVertexArray.length() - 1; CurrentX++)
				{
					#pragma region �w����n�W�誺��� (�W�誺���)
					MyMesh::Point TopLeftPoint, TopRightPoint;
					float  CurrentLeftZ, CurrentRightZ;
					CurrentLeftZ = SideZArray[CurrentZ];
					CurrentRightZ = SideZArray[CurrentZ];

					// �p�G�̥k�䪺�I�A�w�g�W�L�쥻���k��A�N��o�ӥ|��Τ��s�b
					if (lastTopLeftPoint[0] * dx <= TopVertexArray[CurrentX][0] * dx)
						break;
					#pragma endregion
					#pragma region ����n�{�b����� (�U�誺���)
					float NextLeftY, NextRightY, NextLeftZ, NextRightZ;
					NextLeftZ = SideZArray[CurrentZ + 1];
					NextRightZ = SideZArray[CurrentZ + 1];

					NextLeftY = (SideZArray[CurrentZ + 1] * dz > BotVertexArray[CurrentX + 1][2] * dz) ? BotVertexArray[CurrentX + 1][1] :
						(NextLeftZ - TopVertexArray[CurrentX + 1][2]) / (BotVertexArray[CurrentX + 1][2] - TopVertexArray[CurrentX + 1][2]) * (BotVertexArray[CurrentX + 1][1] - TopVertexArray[CurrentX + 1][1]) + TopVertexArray[CurrentX + 1][1];
					NextRightY = (SideZArray[CurrentZ + 1] * dz > BotVertexArray[CurrentX][2] * dz) ? BotVertexArray[CurrentX][1] :
						(NextRightZ - TopVertexArray[CurrentX][2]) / (BotVertexArray[CurrentX][2] - TopVertexArray[CurrentX][2]) * (BotVertexArray[CurrentX][1] - TopVertexArray[CurrentX][1]) + TopVertexArray[CurrentX][1];
					#pragma endregion
					#pragma region ��l��
					// �N��W�誺�I�w�g�W�L�γ��F�A�N��n���L
					if (TopVertexArray[CurrentX][2] * dz > SideZArray[CurrentZ] * dz)
						break;

					MyMesh *tempMesh = new MyMesh;
					SplitCount++;

					// �Ȧs�W�誺�Ҧ��I & �U�誺�I
					QVector<MyMesh::Point> *FinalTopPointArray = new QVector<MyMesh::Point>();
					QVector<MyMesh::Point> *FinalBotPointArray = new QVector<MyMesh::Point>();
					#pragma endregion
					#pragma region �W
					vhandle.clear();

					if (dx > 0)
					{
						// �k�U��(���i��k�U�����I�A�|�]���׽u�Ӵ����ɰ_)
						if (tempPoint[0] * dx < BotVertexArray[CurrentX][0] * dx)
						{
							tempPrograss = (BotVertexArray[CurrentX][0] - PointArray[3][0]) / (PointArray[2][0] - PointArray[3][0]);
							MyMesh::Point tempRightPoint = (PointArray[2] - PointArray[3]) * tempPrograss + PointArray[3];

							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(tempRightPoint)));
							FinalTopPointArray->push_back(MyMesh::Point(tempRightPoint));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(BotVertexArray[CurrentX][0],
								NextRightY, NextRightZ)));
							FinalTopPointArray->push_back(MyMesh::Point(BotVertexArray[CurrentX][0],
								NextRightY, NextRightZ));
						}

						// �k�W��
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(TopVertexArray[CurrentX][0],
							CurrentY, SideZArray[CurrentZ])));
						FinalTopPointArray->push_back(MyMesh::Point(TopVertexArray[CurrentX][0],
							CurrentY, SideZArray[CurrentZ]));

						// ���W��
						if (lastTopLeftPoint[0] * dx >= TopVertexArray[CurrentX + 1][0] * dx)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(TopVertexArray[CurrentX + 1][0],
								CurrentY, SideZArray[CurrentZ])));
							FinalTopPointArray->push_back(MyMesh::Point(TopVertexArray[CurrentX + 1][0],
								CurrentY, SideZArray[CurrentZ]));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(lastTopLeftPoint)));
							FinalTopPointArray->push_back(MyMesh::Point(lastTopLeftPoint));
						}

						// ���U��
						if (tempPoint[2] * dz <= BotVertexArray[CurrentX + 1][2] * dz && tempPoint[0] * dx >= BotVertexArray[CurrentX + 1][0] * dx)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(BotVertexArray[CurrentX + 1][0],
								NextLeftY, NextLeftZ)));
							FinalTopPointArray->push_back(MyMesh::Point(BotVertexArray[CurrentX + 1][0],
								NextLeftY, NextLeftZ));
						}
						else
						{
							// ���
							if (BotVertexArray[CurrentX + 1] != tempPoint && tempPoint[0] * dx >= BotVertexArray[CurrentX][0] * dx)
							{
								// ����Ϊ��P�_
								tempPrograss = (BotVertexArray[CurrentX + 1][0] - PointArray[3][0]) / (PointArray[2][0] - PointArray[3][0]);
								MyMesh::Point tempRightPoint = (PointArray[2] - PointArray[3]) * tempPrograss + PointArray[3];
								if (tempRightPoint != tempPoint && tempRightPoint[1] <= CurrentY && tempRightPoint[1] >= NextLeftY)
								{
									vhandle.push_back(tempMesh->add_vertex(tempRightPoint));
									FinalTopPointArray->push_back(tempRightPoint);
								}

								vhandle.push_back(tempMesh->add_vertex(tempPoint));
								FinalTopPointArray->push_back(tempPoint);
								lastLeftPoint = tempPoint;
							}
						}
					}
					else
					{

						// �k�W��
						if (lastTopLeftPoint[0] * dx >= TopVertexArray[CurrentX + 1][0] * dx)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(TopVertexArray[CurrentX + 1][0],
								CurrentY, SideZArray[CurrentZ])));
							FinalTopPointArray->push_back(MyMesh::Point(TopVertexArray[CurrentX + 1][0],
								CurrentY, SideZArray[CurrentZ]));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(lastTopLeftPoint)));
							FinalTopPointArray->push_back(MyMesh::Point(lastTopLeftPoint));
						}

						// ���W��
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(TopVertexArray[CurrentX][0],
							CurrentY, SideZArray[CurrentZ])));
						FinalTopPointArray->push_back(MyMesh::Point(TopVertexArray[CurrentX][0],
							CurrentY, SideZArray[CurrentZ]));

						// ���U��(���i��k�U�����I�A�|�]���׽u�Ӵ����ɰ_)
						if (tempPoint[0] * dx < BotVertexArray[CurrentX][0] * dx)
						{
							tempPrograss = (BotVertexArray[CurrentX][0] - PointArray[3][0]) / (PointArray[2][0] - PointArray[3][0]);
							MyMesh::Point tempRightPoint = (PointArray[2] - PointArray[3]) * tempPrograss + PointArray[3];

							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(tempRightPoint)));
							FinalTopPointArray->push_back(MyMesh::Point(tempRightPoint));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(BotVertexArray[CurrentX][0],
								NextRightY, NextRightZ)));
							FinalTopPointArray->push_back(MyMesh::Point(BotVertexArray[CurrentX][0],
								NextRightY, NextRightZ));
						}

						// �k�U��
						if (tempPoint[2] * dz <= BotVertexArray[CurrentX + 1][2] * dz && tempPoint[0] * dx >= BotVertexArray[CurrentX + 1][0] * dx)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(BotVertexArray[CurrentX + 1][0],
								NextLeftY, NextLeftZ)));
							FinalTopPointArray->push_back(MyMesh::Point(BotVertexArray[CurrentX + 1][0],
								NextLeftY, NextLeftZ));
						}
						else
						{
							// ���
							if (BotVertexArray[CurrentX + 1] != tempPoint && tempPoint[0] * dx >= BotVertexArray[CurrentX][0] * dx)
							{

								vhandle.push_back(tempMesh->add_vertex(tempPoint));
								FinalTopPointArray->push_back(tempPoint);
								lastLeftPoint = tempPoint;

								// ����Ϊ��P�_
								tempPrograss = (BotVertexArray[CurrentX + 1][0] - PointArray[3][0]) / (PointArray[2][0] - PointArray[3][0]);
								MyMesh::Point tempRightPoint = (PointArray[2] - PointArray[3]) * tempPrograss + PointArray[3];
								if (tempRightPoint != tempPoint && tempRightPoint[1] <= CurrentY && tempRightPoint[1] >= NextLeftY)
								{
									vhandle.push_back(tempMesh->add_vertex(tempRightPoint));
									FinalTopPointArray->push_back(tempRightPoint);
								}
							}
						}
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U
					vhandle.clear();

					// �����ͤU�����I
					for (int j = 0; j < FinalTopPointArray->length(); j++)
					{
						// �Ȧs����2�� Point
						MyMesh::Point tempPoint2;
						if ((*FinalTopPointArray)[j][1] <= MinHeight)
							tempPoint2 = (*FinalTopPointArray)[j];
						else
							tempPoint2 = (*FinalTopPointArray)[j] - MyMesh::Point(0, Height, 0);
						FinalBotPointArray->push_back(tempPoint2);
					}

					// ���F�b���Y�_�Ӫ��γ��A�ҥH�[�o���ܼ�
					EndPointVector.push_back((*FinalBotPointArray)[FinalBotPointArray->length() - 1]);

					// ���ͭ�(�]���o�� Normal �¤W�A�ҥH�n�f�ɥͲ����I)
					for (int j = FinalBotPointArray->length() - 1; j >= 0; j--)
						vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[j]));
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ��L����
					for (int j = 0; j < FinalTopPointArray->length() - 1; j++)
					{
						vhandle.clear();

						vhandle.push_back(tempMesh->add_vertex((*FinalTopPointArray)[j + 1]));
						vhandle.push_back(tempMesh->add_vertex((*FinalTopPointArray)[j]));
						vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[j]));
						vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[j + 1]));

						tempMesh->add_face(vhandle.toStdVector());
					}

					// �A�ɤ@�� 0 �� �̫�@�Ӧa�s�u
					vhandle.clear();

					vhandle.push_back(tempMesh->add_vertex((*FinalTopPointArray)[0]));
					vhandle.push_back(tempMesh->add_vertex((*FinalTopPointArray)[FinalTopPointArray->length() - 1]));
					vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[FinalTopPointArray->length() - 1]));
					vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[0]));

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					delete FinalTopPointArray;
					delete FinalBotPointArray;
				}

				//��̥~�����I�[�i�h
				if (RDPoint.length() == 0 || RDPoint[RDPoint.length() - 1] != EndPointVector[EndPointVector.length() - 1])
					RDPoint.push_back(EndPointVector[EndPointVector.length() - 1]);
				else if (RDPoint.length() != 0 && RDPoint[RDPoint.length() - 1] == EndPointVector[EndPointVector.length() - 1])
					RDPoint.push_back(EndPointVector[EndPointVector.length() - 2]);
				lastTopLeftPoint = lastLeftPoint;
			}
			#pragma endregion
			#pragma region �[���� info & �@�Ǹ�T(����)
			TopVertexArray.clear();
			BotVertexArray.clear();
			SideZArray.clear();

			// ����X Top �I�� Array
			dx = (PointArray[7][0] - PointArray[6][0] > 0) ? 1 : -1;
			dz = (PointArray[5][2] - PointArray[6][2] > 0) ? 1 : -1;

			// ����X Bottom �I�� Array
			MidX = PointArray[7][0];
			PointArray[3][0] = MidX;		// �ץ��~�t�A�쥻�� X �b��bug

			CurrentX = PointArray[5][0];
			EndX = PointArray[3][0];
			GoSlash = false;

			// X �b
			while (CurrentX * dx < EndX * dx)
			{
				NextX = GetNextValue(CurrentX, dx, EndX);

				if (CurrentX * dx >= MidX * dx)
				{
					float prograss = (CurrentX - PointArray[3][0]) / (PointArray[2][0] - PointArray[3][0]);
					tempPoint = (PointArray[2] - PointArray[3]) * prograss + PointArray[3];

					// Bot ���I�A�p�G�@�}�l���O 0 ���ܤ~�n�[
					if (prograss != 0 && prograss != 1 && !GoSlash)
					{
						TopVertexArray.push_back(MyMesh::Point(PointArray[2][0], PointArray[0][1], PointArray[0][2]));
						BotVertexArray.push_back(PointArray[2]);
					}
					GoSlash = true;

					TopVertexArray.push_back(MyMesh::Point(CurrentX, PointArray[0][1], PointArray[0][2]));
					BotVertexArray.push_back(tempPoint);
				}
				else
				{
					TopVertexArray.push_back(MyMesh::Point(CurrentX, PointArray[0][1], PointArray[0][2]));
					BotVertexArray.push_back(MyMesh::Point(CurrentX, PointArray[1][1], PointArray[1][2]));
				}

				CurrentX = NextX;
			}
			// �P�_�O�_���A�̩w�~�t�H���A�p�G�S���A�N��n���I�[�i��
			if (abs((TopVertexArray[TopVertexArray.length() - 1][0] - NextX)) > 0.01f)
			{
				TopVertexArray.push_back(PointArray[7]);
				BotVertexArray.push_back(PointArray[3]);
			}

			// Z �b
			MidZ = PointArray[3][2];
			CurrentZ = (PointArray[6][2] < 0.01f) ? 0 : PointArray[6][2];;
			EndZ = PointArray[5][2];
			GoSlash = false;
			
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);

				if (CurrentZ * dz >= MidZ * dz)
				{
					float prograss = (CurrentZ - PointArray[3][2]) / (PointArray[4][2] - PointArray[3][2]);
					tempPoint = (PointArray[2] - PointArray[3]) * prograss + PointArray[3];

					// Bot ���I�A�p�G�@�}�l���O 0 ���ܤ~�n�[
					if (prograss != 0 && !GoSlash)
						SideZArray.push_back(PointArray[3][2]);
					GoSlash = true;

					SideZArray.push_back(tempPoint[2]);
				}
				else
					SideZArray.push_back(CurrentZ);

				CurrentZ = NextZ;
			}
			// �P�_�O�_���A�̩w�~�t�H���A�p�G�S���A�N��n���I�[�i��
			if (abs((SideZArray[SideZArray.length() - 1] - NextZ)) > 0.01f)
				SideZArray.push_back(PointArray[2][2]);
			#pragma endregion
			#pragma region �[�F��i�}�C��(����)
			lastTopLeftPoint = MyMesh::Point(0,0,0);	// For �W��A�n�^���I���ɭԦa���s�Ϊ�	

			for (CurrentZ = 0; CurrentZ < SideZArray.length() - 1; CurrentZ++)
			{
				float prograss = (SideZArray[CurrentZ] - BotVertexArray[0][2]) / (TopVertexArray[0][2] - BotVertexArray[0][2]);
				float CurrentY = prograss * (TopVertexArray[0][1] - BotVertexArray[0][1]) + BotVertexArray[0][1];

				// �Ȧ���Ϊ��s�b
				float tempPrograss = (SideZArray[CurrentZ + 1] - PointArray[3][2]) / (PointArray[4][2] - PointArray[3][2]);
				tempPoint = (PointArray[4] - PointArray[3]) * tempPrograss + PointArray[3];

				MyMesh::Point lastLeftPoint(0, 0, 0);
				QVector<MyMesh::Point> EndPointVector;
				for (CurrentX = 0; CurrentX < TopVertexArray.length() - 1; CurrentX++)
				{
					#pragma region �w����n�W�誺��� (�W�誺���)
					MyMesh::Point TopLeftPoint, TopRightPoint;
					float  CurrentLeftZ, CurrentRightZ;
					CurrentLeftZ = SideZArray[CurrentZ];
					CurrentRightZ = SideZArray[CurrentZ];

					// �p�G�̥k�䪺�I�A�w�g�W�L�쥻���k��A�N��o�ӥ|��Τ��s�b
					if (lastTopLeftPoint[0] * dx <= TopVertexArray[CurrentX][0] * dx)
						break;
					#pragma endregion
					#pragma region ����n�{�b����� (�U�誺���)
					float NextLeftY, NextRightY, NextLeftZ, NextRightZ;
					NextLeftZ = SideZArray[CurrentZ + 1];
					NextRightZ = SideZArray[CurrentZ + 1];

					NextLeftY = (SideZArray[CurrentZ + 1] * dz > BotVertexArray[CurrentX + 1][2] * dz) ? BotVertexArray[CurrentX + 1][1] :
						(NextLeftZ - TopVertexArray[CurrentX + 1][2]) / (BotVertexArray[CurrentX + 1][2] - TopVertexArray[CurrentX + 1][2]) * (BotVertexArray[CurrentX + 1][1] - TopVertexArray[CurrentX + 1][1]) + TopVertexArray[CurrentX + 1][1];
					NextRightY = (SideZArray[CurrentZ + 1] * dz > BotVertexArray[CurrentX][2] * dz) ? BotVertexArray[CurrentX][1] :
						(NextRightZ - TopVertexArray[CurrentX][2]) / (BotVertexArray[CurrentX][2] - TopVertexArray[CurrentX][2]) * (BotVertexArray[CurrentX][1] - TopVertexArray[CurrentX][1]) + TopVertexArray[CurrentX][1];
					#pragma endregion
					#pragma region ��l��
					// �N��W�誺�I�w�g�W�L�γ��F�A�N��n���L
					if (TopVertexArray[CurrentX][2] * dz > SideZArray[CurrentZ] * dz)
						break;

					MyMesh *tempMesh = new MyMesh;
					SplitCount++;

					// �Ȧs�W�誺�Ҧ��I & �U�誺�I
					QVector<MyMesh::Point> *FinalTopPointArray = new QVector<MyMesh::Point>();
					QVector<MyMesh::Point> *FinalBotPointArray = new QVector<MyMesh::Point>();
					#pragma endregion
					#pragma region �W
					vhandle.clear();

					if (dx > 0)
					{
						// �k�U��(���i��k�U�����I�A�|�]���׽u�Ӵ����ɰ_)
						if (tempPoint[0] * dx < BotVertexArray[CurrentX][0] * dx)
						{
							tempPrograss = (BotVertexArray[CurrentX][0] - PointArray[3][0]) / (PointArray[4][0] - PointArray[3][0]);
							MyMesh::Point tempRightPoint = (PointArray[4] - PointArray[3]) * tempPrograss + PointArray[3];

							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(tempRightPoint)));
							FinalTopPointArray->push_back(MyMesh::Point(tempRightPoint));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(BotVertexArray[CurrentX][0],
								NextRightY, NextRightZ)));
							FinalTopPointArray->push_back(MyMesh::Point(BotVertexArray[CurrentX][0],
								NextRightY, NextRightZ));
						}

						// �k�W��
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(TopVertexArray[CurrentX][0],
							CurrentY, SideZArray[CurrentZ])));
						FinalTopPointArray->push_back(MyMesh::Point(TopVertexArray[CurrentX][0],
							CurrentY, SideZArray[CurrentZ]));

						// ���W��
						if (lastTopLeftPoint[0] * dx >= TopVertexArray[CurrentX + 1][0] * dx)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(TopVertexArray[CurrentX + 1][0],
								CurrentY, SideZArray[CurrentZ])));
							FinalTopPointArray->push_back(MyMesh::Point(TopVertexArray[CurrentX + 1][0],
								CurrentY, SideZArray[CurrentZ]));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(lastTopLeftPoint)));
							FinalTopPointArray->push_back(MyMesh::Point(lastTopLeftPoint));
						}

						// ���U��
						if (tempPoint[2] * dz <= BotVertexArray[CurrentX + 1][2] * dz && tempPoint[0] * dx >= BotVertexArray[CurrentX + 1][0] * dx)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(BotVertexArray[CurrentX + 1][0],
								NextLeftY, NextLeftZ)));
							FinalTopPointArray->push_back(MyMesh::Point(BotVertexArray[CurrentX + 1][0],
								NextLeftY, NextLeftZ));
						}
						else
						{
							// ���
							if (BotVertexArray[CurrentX + 1] != tempPoint && tempPoint[0] * dx >= BotVertexArray[CurrentX][0] * dx)
							{
								// ����Ϊ��P�_
								tempPrograss = (BotVertexArray[CurrentX + 1][0] - PointArray[3][0]) / (PointArray[4][0] - PointArray[3][0]);
								MyMesh::Point tempRightPoint = (PointArray[4] - PointArray[3]) * tempPrograss + PointArray[3];
								if (tempRightPoint != tempPoint && tempRightPoint[1] <= CurrentY && tempRightPoint[1] >= NextLeftY)
								{
									vhandle.push_back(tempMesh->add_vertex(tempRightPoint));
									FinalTopPointArray->push_back(tempRightPoint);
								}

								vhandle.push_back(tempMesh->add_vertex(tempPoint));
								FinalTopPointArray->push_back(tempPoint);
								lastLeftPoint = tempPoint;
							}
						}
					}
					else
					{
						// �k�W��
						if (lastTopLeftPoint[0] * dx >= TopVertexArray[CurrentX + 1][0] * dx)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(TopVertexArray[CurrentX + 1][0],
								CurrentY, SideZArray[CurrentZ])));
							FinalTopPointArray->push_back(MyMesh::Point(TopVertexArray[CurrentX + 1][0],
								CurrentY, SideZArray[CurrentZ]));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(lastTopLeftPoint)));
							FinalTopPointArray->push_back(MyMesh::Point(lastTopLeftPoint));
						}

						// ���W��
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(TopVertexArray[CurrentX][0],
							CurrentY, SideZArray[CurrentZ])));
						FinalTopPointArray->push_back(MyMesh::Point(TopVertexArray[CurrentX][0],
							CurrentY, SideZArray[CurrentZ]));

						// ���U��(���i�४�U�����I�A�|�]���׽u�Ӵ����ɰ_)
						if (tempPoint[0] * dx < BotVertexArray[CurrentX][0] * dx)
						{
							tempPrograss = (BotVertexArray[CurrentX][0] - PointArray[3][0]) / (PointArray[4][0] - PointArray[3][0]);
							MyMesh::Point tempRightPoint = (PointArray[4] - PointArray[3]) * tempPrograss + PointArray[3];

							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(tempRightPoint)));
							FinalTopPointArray->push_back(MyMesh::Point(tempRightPoint));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(BotVertexArray[CurrentX][0],
								NextRightY, NextRightZ)));
							FinalTopPointArray->push_back(MyMesh::Point(BotVertexArray[CurrentX][0],
								NextRightY, NextRightZ));
						}

						// �k�U��
						if (tempPoint[2] * dz <= BotVertexArray[CurrentX + 1][2] * dz && tempPoint[0] * dx >= BotVertexArray[CurrentX + 1][0] * dx)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(BotVertexArray[CurrentX + 1][0],
								NextLeftY, NextLeftZ)));
							FinalTopPointArray->push_back(MyMesh::Point(BotVertexArray[CurrentX + 1][0],
								NextLeftY, NextLeftZ));
						}
						else
						{
							// ���
							if (BotVertexArray[CurrentX + 1] != tempPoint && tempPoint[0] * dx >= BotVertexArray[CurrentX][0] * dx)
							{
								vhandle.push_back(tempMesh->add_vertex(tempPoint));
								FinalTopPointArray->push_back(tempPoint);
								lastLeftPoint = tempPoint;

								// ����Ϊ��P�_
								tempPrograss = (BotVertexArray[CurrentX + 1][0] - PointArray[3][0]) / (PointArray[4][0] - PointArray[3][0]);
								MyMesh::Point tempRightPoint = (PointArray[4] - PointArray[3]) * tempPrograss + PointArray[3];
								if (tempRightPoint != tempPoint && tempRightPoint[1] <= CurrentY && tempRightPoint[1] >= NextLeftY)
								{
									vhandle.push_back(tempMesh->add_vertex(tempRightPoint));
									FinalTopPointArray->push_back(tempRightPoint);
								}
							}
						}
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U
					vhandle.clear();

					// �����ͤU�����I
					for (int j = 0; j < FinalTopPointArray->length(); j++)
					{
						// �Ȧs����2�� Point
						MyMesh::Point tempPoint2;
						if ((*FinalTopPointArray)[j][1] <= MinHeight)
							tempPoint2 = (*FinalTopPointArray)[j];
						else
							tempPoint2 = (*FinalTopPointArray)[j] - MyMesh::Point(0, Height, 0);
						FinalBotPointArray->push_back(tempPoint2);
					}

					// ���F�b���Y�_�Ӫ��γ��A�ҥH�[�o���ܼ�
					EndPointVector.push_back((*FinalBotPointArray)[FinalBotPointArray->length() - 1]);

					// ���ͭ�(�]���o�� Normal �¤W�A�ҥH�n�f�ɥͲ����I)
					for (int j = FinalBotPointArray->length() - 1; j >= 0; j--)
						vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[i]));
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ��L����
					for (int j = 0; j < FinalTopPointArray->length() - 1; j++)
					{
						vhandle.clear();

						vhandle.push_back(tempMesh->add_vertex((*FinalTopPointArray)[j + 1]));
						vhandle.push_back(tempMesh->add_vertex((*FinalTopPointArray)[j]));
						vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[j]));
						vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[j + 1]));

						tempMesh->add_face(vhandle.toStdVector());
					}

					// �A�ɤ@�� 0 �� �̫�@�Ӧa�s�u
					vhandle.clear();

					vhandle.push_back(tempMesh->add_vertex((*FinalTopPointArray)[0]));
					vhandle.push_back(tempMesh->add_vertex((*FinalTopPointArray)[FinalTopPointArray->length() - 1]));
					vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[FinalTopPointArray->length() - 1]));
					vhandle.push_back(tempMesh->add_vertex((*FinalBotPointArray)[0]));

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					delete FinalTopPointArray;
					delete FinalBotPointArray;
				}

				//��̥~�����I�[�i�h
				if (LDPoint .length() == 0 || LDPoint[LDPoint.length() - 1] != EndPointVector[EndPointVector.length() - 1])
					LDPoint.push_back(EndPointVector[EndPointVector.length() - 1]);
				else if (LDPoint.length() != 0 && LDPoint[LDPoint.length() - 1] == EndPointVector[EndPointVector.length() - 1])
					LDPoint.push_back(EndPointVector[EndPointVector.length() - 2]);
				lastTopLeftPoint = lastLeftPoint;
			}
			#pragma endregion
			#pragma region ��F�������iSplit�̭�
			if ((SplitCount - lastSplitCount) != 0)
			{
				splitInfo->SplitCount = SplitCount - lastSplitCount;
				lastSplitCount = SplitCount;
				SplitInfoArray.push_back(splitInfo);
			}
			#pragma endregion
			#pragma endregion
			#pragma region �Y�_�Ӫ��a��(���]�t�T����)
			//�ᱼ�̥~�����I
			LDPoint.pop_back();
			RDPoint.pop_back();

			#pragma region �k��
			#pragma region �[�I�i�h
			offsetArray.clear();
			offsetArray = { 20, 8, 6, 5, 0, 9, 7};

			PointArray.clear();
			for (int j = 0; j < offsetArray.size(); j++)
			{
				MyMesh::Point tempP;
				if (offsetArray[j] == 20)
				{
					tempP = ModelsArray[i + 1]->point(v1_it + offsetArray[j]);
					tempP[0] = 0;
				}
				else if (offsetArray[j] == 0)
					tempP = PointArray[j - 4] - MyMesh::Point(0, Height, 0);
				else
					tempP = ModelsArray[i + 2]->point(v2_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}

			dx = PointArray[2][0] - PointArray[1][0] < 0 ? -1 : 1;
			#pragma endregion
			#pragma region �[����A�ð�����
			splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i + 2;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i + 2]->PartNumber;
			splitInfo->PartName = InfoArray[i + 2]->PartName;

			// ������
			QVector<MyMesh::Point> FinalTopVertex;
			for (int j = 1; j < RDPoint.length(); j++)
			{
				tempPoint = MyMesh::Point(PointArray[1][0], PointArray[1][1], RDPoint[j][2]);
				FinalTopVertex.push_back(tempPoint);
			}

			for (int j = 0; j < RDPoint.length() - 1; j++)
			{
				MyMesh *tempMesh = new MyMesh;
				SplitCount++;
				
				#pragma region �k��
				vhandle.clear();
				if (dx < 0)
				{
					if (j == 0)
						vhandle.push_back(tempMesh->add_vertex(PointArray[0]));
					else
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));

					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j] + MyMesh::Point(0, Height, 0)));
				}
				else
				{
					if (j == 0)
						vhandle.push_back(tempMesh->add_vertex(PointArray[0]));
					else
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));

					vhandle.push_back(tempMesh->add_vertex(RDPoint[j] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
				}

				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region ����
				vhandle.clear();
				if (dx < 0)
				{
					if (j == 0)
						vhandle.push_back(tempMesh->add_vertex(PointArray[4]));
					else
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));

					vhandle.push_back(tempMesh->add_vertex(RDPoint[j]));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
				}
				else
				{
					if (j == 0)
						vhandle.push_back(tempMesh->add_vertex(PointArray[4]));
					else
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));

					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j]));
				}

				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �W��
				vhandle.clear();
				if (dx < 0)
				{
					if (j == 0)
					{
						vhandle.push_back(tempMesh->add_vertex(PointArray[0]));
						vhandle.push_back(tempMesh->add_vertex(PointArray[4]));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));
					}

					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
				}
				else
				{
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));

					if (j == 0)
					{
						vhandle.push_back(tempMesh->add_vertex(PointArray[4]));
						vhandle.push_back(tempMesh->add_vertex(PointArray[0]));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));
					}
				}
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �U��
				vhandle.clear();
				
				if (dx < 0)
				{
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j]));
				}
				else
				{
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j]));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j] + MyMesh::Point(0, Height, 0)));
				}

				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �e��
				vhandle.clear();

				if (dx < 0)
				{
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
				}
				else
				{
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(RDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
				}

				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �᭱
				if (j != 0)
				{
					vhandle.clear();
					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));
						vhandle.push_back(tempMesh->add_vertex(RDPoint[j] + MyMesh::Point(0, Height, 0)));
						vhandle.push_back(tempMesh->add_vertex(RDPoint[j]));
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));
						vhandle.push_back(tempMesh->add_vertex(RDPoint[j]));
						vhandle.push_back(tempMesh->add_vertex(RDPoint[j] + MyMesh::Point(0, Height, 0)));
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));
					}

					tempMesh->add_face(vhandle.toStdVector());
				}
				#pragma endregion

				tempMesh->update_normals();
				SplitModelsArray.push_back(tempMesh);
			}

			#pragma region �W�[�̫e�������@��
			MyMesh *tempMesh = new MyMesh;
			SplitCount++;

			#pragma region �k��
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region ����
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �W��
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �U��
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �_�Ǫ��@�Ө�
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �e��
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �᭱
			vhandle.clear();

			if(dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(RDPoint[RDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			
			tempMesh->update_normals();
			SplitModelsArray.push_back(tempMesh);
			#pragma endregion
			#pragma endregion
			#pragma endregion
			#pragma region ����
			#pragma region �[�I�i�h
			offsetArray.clear();
			offsetArray = { 20, 8, 1, 2, 0, 9, -1};

			PointArray.clear();
			for (int j = 0; j < offsetArray.size(); j++)
			{
				MyMesh::Point tempP;
				if (offsetArray[j] == 20)
				{
					tempP = ModelsArray[i + 1]->point(v1_it + offsetArray[j]);
					tempP[0] = 0;
				}
				else if (offsetArray[j] == 0)
					tempP = PointArray[j - 4] - MyMesh::Point(0, Height, 0);
				else if (offsetArray[j] == -1)
					tempP = ModelsArray[i + 2]->point(v2_it);
				else
					tempP = ModelsArray[i + 2]->point(v2_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}

			dx = PointArray[2][0] - PointArray[1][0] < 0 ? -1 : 1;
			#pragma endregion
			#pragma region �[����A�ð�����
			// ������
			FinalTopVertex.clear();
			for (int j = 1; j < LDPoint.length(); j++)
			{
				tempPoint = MyMesh::Point(PointArray[1][0], PointArray[1][1], RDPoint[j][2]);
				FinalTopVertex.push_back(tempPoint);
			}

			for (int j = 0; j < LDPoint.length() - 1; j++)
			{
				MyMesh *tempMesh = new MyMesh;
				SplitCount++;
				
				#pragma region �k��
				vhandle.clear();
				if (dx < 0)
				{
					if (j == 0)
						vhandle.push_back(tempMesh->add_vertex(PointArray[0]));
					else
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));

					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j] + MyMesh::Point(0, Height, 0)));
				}
				else
				{
					if (j == 0)
						vhandle.push_back(tempMesh->add_vertex(PointArray[0]));
					else
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));

					vhandle.push_back(tempMesh->add_vertex(LDPoint[j] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
				}

				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region ����
				vhandle.clear();
				if (dx < 0)
				{
					if (j == 0)
						vhandle.push_back(tempMesh->add_vertex(PointArray[4]));
					else
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));

					vhandle.push_back(tempMesh->add_vertex(LDPoint[j]));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
				}
				else
				{
					if (j == 0)
						vhandle.push_back(tempMesh->add_vertex(PointArray[4]));
					else
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));

					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j]));
				}

				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �W��
				vhandle.clear();
				if (dx < 0)
				{
					if (j == 0)
					{
						vhandle.push_back(tempMesh->add_vertex(PointArray[0]));
						vhandle.push_back(tempMesh->add_vertex(PointArray[4]));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));
					}

					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
				}
				else
				{
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));

					if (j == 0)
					{
						vhandle.push_back(tempMesh->add_vertex(PointArray[4]));
						vhandle.push_back(tempMesh->add_vertex(PointArray[0]));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));
					}
				}
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �U��
				vhandle.clear();
				
				if (dx < 0)
				{
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j]));
				}
				else
				{
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j]));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j] + MyMesh::Point(0, Height, 0)));
				}

				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �e��
				vhandle.clear();

				if (dx < 0)
				{
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
				}
				else
				{
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1] + MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(LDPoint[j + 1]));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j] - MyMesh::Point(0, Height, 0)));
					vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j]));
				}

				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �᭱
				if (j != 0)
				{
					vhandle.clear();
					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));
						vhandle.push_back(tempMesh->add_vertex(LDPoint[j] + MyMesh::Point(0, Height, 0)));
						vhandle.push_back(tempMesh->add_vertex(LDPoint[j]));
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1] - MyMesh::Point(0, Height, 0)));
						vhandle.push_back(tempMesh->add_vertex(LDPoint[j]));
						vhandle.push_back(tempMesh->add_vertex(LDPoint[j] + MyMesh::Point(0, Height, 0)));
						vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[j - 1]));
					}

					tempMesh->add_face(vhandle.toStdVector());
				}
				#pragma endregion

				tempMesh->update_normals();
				SplitModelsArray.push_back(tempMesh);
			}

			#pragma region �W�[�̫e�������@��
			tempMesh = new MyMesh;
			SplitCount++;
			#pragma region �k��
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region ����
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �W��
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �U��
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �_�Ǫ��@�Ө�
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[3]));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �e��
			vhandle.clear();

			if (dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(PointArray[2]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[6]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[5]));
				vhandle.push_back(tempMesh->add_vertex(PointArray[1]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion
			#pragma region �᭱
			vhandle.clear();

			if(dx < 0)
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
			}
			else
			{
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1] - MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1]));
				vhandle.push_back(tempMesh->add_vertex(LDPoint[LDPoint.length() - 1] + MyMesh::Point(0, Height, 0)));
				vhandle.push_back(tempMesh->add_vertex(FinalTopVertex[FinalTopVertex.length() - 1]));
			}

			tempMesh->add_face(vhandle.toStdVector());
			#pragma endregion


			tempMesh->update_normals();
			SplitModelsArray.push_back(tempMesh);
			#pragma endregion
			#pragma endregion
			#pragma endregion

			// �[�i split Info ��
			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;
			SplitInfoArray.push_back(splitInfo);
			#pragma endregion

			// �]���쥻���γ��S���g�n�A�ҥH�@�����L�T��
			i += 3;
		}
		#pragma endregion
		#pragma region �a�O & �������
		// �a�O & �S�����᪺��
		else if (InfoArray[i]->PartName.endsWith("/basic") || InfoArray[i]->PartName.endsWith("/no_window"))
		{
			#pragma region 	�ư����~���p
			if (ModelsArray[i]->n_vertices() != 24)
			{
				cout << "Base �� Wall �����~" << endl;
				return;
			}
			#pragma endregion
			#pragma region ��X�̥~�򪺨���I
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			int *offsetArray = new int[2];
			if (InfoArray[i]->PartName.contains("Wall") && InfoArray[i]->PartName != "backWall")
			{
				offsetArray[0] = 21;
				offsetArray[1] = 0;
			}
			else
			{
				offsetArray[0] = 0;
				offsetArray[1] = 21;
			}

			for (int j = 0; j < 2; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region �}�l�n�[����
			// 0 �������A��n�����̶]
			float dx = (PointArray[1][0] - PointArray[0][0] > 0) ? 1 : -1;
			float dy = (PointArray[1][1] - PointArray[0][1] > 0) ? 1 : -1;
			float dz = (PointArray[1][2] - PointArray[0][2] > 0) ? 1 : -1;

			// �q�o�Ӷ}�l
			float CurrentX;
			float CurrentY;
			float CurrentZ;

			// �U�@�Ӫ� X, Y, Z
			float NextX;
			float NextY;
			float NextZ;

			// �}�l����
			float StartX = PointArray[0][0];
			float StartY = PointArray[0][1];
			float StartZ = PointArray[0][2];
			
			// ��������
			float EndX = PointArray[1][0];
			float EndY = PointArray[1][1];
			float EndZ = PointArray[1][2];

			// Bool �O�_���Ԩ�̫᭱
			bool IsEndX = false, IsEndY = false, IsEndZ = false;

			// �s�W info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;
			
			// �s�W�d�g��T
			CountInfo countInfo;
			countInfo.XCount = CountSize(StartX, EndX);
			countInfo.YCount = CountSize(StartY, EndY);
			countInfo.ZCount = CountSize(StartZ, EndZ);
			countInfo.XDir = dx;
			countInfo.YDir = dy;
			countInfo.ZDir = dz;
			splitInfo->LockDataInfo.push_back(countInfo);

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);
				CurrentY = StartY;
				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);
					CurrentX = StartX;
					while (CurrentX * dx < EndX * dx)
					{
						NextX = GetNextValue(CurrentX, dx, EndX);
						#pragma region ��l��
						MyMesh *tempMesh = new MyMesh();
						SplitCount++;
						#pragma endregion
						#pragma region �N�I�[�W�h
						#pragma region �W
						vhandle.clear();
						if (dx * dy * dz < 0)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion
						#pragma region �U
						vhandle.clear();
						if (dx * dy * dz < 0)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion
						#pragma region ��
						vhandle.clear();
						if (dx * dy * dz < 0)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion
						#pragma region �k
						vhandle.clear();
						if (dx * dy * dz < 0)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion
						#pragma region �e
						vhandle.clear();
						if (dx * dy * dz < 0)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion
						#pragma region ��
						vhandle.clear();
						if (dx * dy * dz < 0)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion

						tempMesh->update_normals();
						SplitModelsArray.push_back(tempMesh);
						#pragma endregion
						CurrentX = NextX;
					}
					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}
			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;

			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
			#pragma region �M��
			delete[] offsetArray;
			#pragma endregion
		}
		
		// ���@�Ӫ�
		else if (InfoArray[i]->PartName.endsWith("/door_entry"))
		{
			#pragma region 	�ư����~���p
			if (ModelsArray[i]->n_vertices() != 56)
			{
				cout << "Door Entry �����~" << endl;
				return;
			}
			#pragma endregion
			#pragma region �[�i8���I x 2
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			int offsetArray[] = { 0,1,2,29,45,42,26,3,5,4,7,28,39,38,27,6 };
			for (int j = 0; j < 16; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region �}�l�n�[����
			QVector<MyMesh::VertexHandle>	vhandle;						// Vertex Handle

			// �s�W info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;

			// 0 �������A��n�����̶]
			float dy = (PointArray[7][2] - PointArray[0][2]) < 0 ? -1 : 1;
			float dz = (PointArray[1][2] - PointArray[0][2]) < 0 ? -1 : 1;

			// �q�o�Ӷ}�l
			float CurrentY;
			float CurrentZ;

			// �U�@�Ӫ� Y, Z
			float NextY;
			float NextZ;

			// �}�l����
			float StartY = PointArray[0][1];
			float StartZ = PointArray[0][2];
			
			// ��������
			float EndY = PointArray[7][1];
			float EndZ = PointArray[1][2];

			// �첾�q
			float offsetX = PointArray[0][0] - PointArray[8][0];

			//�Ĥ@�q
			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ���
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}

			//�ĤG�q
			StartY = PointArray[2][1];
			StartZ = PointArray[2][2];
			EndZ = PointArray[3][2];

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ���
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}

			// �ĤT�q
			StartY = PointArray[4][1];
			StartZ = PointArray[4][2];
			EndZ = PointArray[5][2];

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ���
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}

			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;
			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
		}

		// ���@�ӵ�
		else if (InfoArray[i]->PartName.endsWith("/single_window"))
		{
			#pragma region 	�ư����~���p
			if (ModelsArray[i]->n_vertices() != 64)
			{
				cout << "Single Window �����~" << endl;
				return;
			}
			#pragma endregion
			#pragma region �[�i8���I x 2
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			//int offsetArray[] = { 23,7, 6,39, 20,4, 5,36, 16,0, 1,32, 19,3, 2,35 };
			int offsetArray[] = { 7,23, 39,6, 4,20, 36,5, 0,16, 32,1, 3,19, 35,2 };
			for (int j = 0; j < 16; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region �}�l�n�[����
			QVector<MyMesh::VertexHandle>	vhandle;						// Vertex Handle

			// �s�W info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;

			// 0 �������A��n�����̶]
			float dy = (PointArray[3][2] - PointArray[0][2]) < 0 ? -1 : 1;
			float dz = (PointArray[1][2] - PointArray[0][2]) < 0 ? -1 : 1;

			// �q�o�Ӷ}�l
			float CurrentY;
			float CurrentZ;

			// �U�@�Ӫ� Y, Z
			float NextY;
			float NextZ;

			// �}�l����
			float StartY = PointArray[0][1];
			float StartZ = PointArray[0][2];
			
			// ��������
			float EndY = PointArray[3][1];
			float EndZ = PointArray[4][2];

			// �첾�q
			float offsetX = PointArray[0][0] - PointArray[8][0];

			//�Ĥ@�q
			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ���
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}

			//�ĤG�q
			StartY = PointArray[0][1];
			StartZ = PointArray[4][2];
			EndY = PointArray[4][1];
			EndZ = PointArray[5][2];

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ���
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}

			// �ĤT�q
			StartY = PointArray[7][1];
			StartZ = PointArray[7][2];
			EndY = PointArray[3][1];
			EndZ = PointArray[6][2];

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ���
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}

			// �ĥ|�q
			StartY = PointArray[0][1];
			StartZ = PointArray[5][2];
			EndY = PointArray[2][1];
			EndZ = PointArray[1][2];

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ���
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[8][0], CurrentY, CurrentZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}

			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;
			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
		}

		// �h�ӵ���
		else if (InfoArray[i]->PartName.endsWith("/multi_window"))
		{
			#pragma region 	�ư����~���p
			if (ModelsArray[i]->n_vertices() < 155)
			{
				cout << "Multi Window �����~" << endl;
				return;
			}
			#pragma endregion
			#pragma region �[�i12���I x 2
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			int offsetArray[] = { 44,45,55,53,49,43,51,57,42,73,75,52,5,0,9,13,31,2,10,17,1,7,35,11 };
			for (int j = 0; j < 24; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region �}�l�n�[����
			QVector<MyMesh::VertexHandle>	vhandle;						// Vertex Handle

			// �s�W info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;

			// 0 �������A��n�����̶]
			float dx = (PointArray[1][0] - PointArray[0][0]) < 0 ? -1 : 1;
			float dy = (PointArray[3][1] - PointArray[0][1]) < 0 ? -1 : 1;

			// �q�o�Ӷ}�l
			float CurrentX;
			float CurrentY;

			// �U�@�Ӫ� Y, Z
			float NextX;
			float NextY;

			// �}�l����
			float StartX = PointArray[0][0];
			float StartY = PointArray[0][1];
			
			// ��������
			float EndX = PointArray[4][0];
			float EndY = PointArray[3][1];

			// �첾�q
			float offsetZ = PointArray[12][2] - PointArray[0][2];

			// �Ĥ@�q
			CurrentX = StartX;
			while (CurrentX * dx < EndX * dx)
			{
				NextX = GetNextValue(CurrentX, dx, EndX);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �᭱
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion

					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentX = NextX;
			}

			// �ĤG�q
			StartX = PointArray[4][0];
			StartY = PointArray[0][1];
			EndX = PointArray[5][0];
			EndY = PointArray[4][1];

			CurrentX = StartX;
			while (CurrentX * dx < EndX * dx)
			{
				NextX = GetNextValue(CurrentX, dx, EndX);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �᭱
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion


					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentX = NextX;
			}

			// �ĤT�q
			StartX = PointArray[7][0];
			StartY = PointArray[7][1];
			EndX = PointArray[6][0];
			EndY = PointArray[3][1];

			CurrentX = StartX;
			while (CurrentX * dx < EndX * dx)
			{
				NextX = GetNextValue(CurrentX, dx, EndX);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �᭱
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion


					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentX = NextX;
			}

			// �ĥ|�q
			StartX = PointArray[5][0];
			StartY = PointArray[0][1];
			EndX = PointArray[8][0];
			EndY = PointArray[3][1];

			CurrentX = StartX;
			while (CurrentX * dx < EndX * dx)
			{
				NextX = GetNextValue(CurrentX, dx, EndX);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �᭱
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion


					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentX = NextX;
			}

			// �Ĥ��q
			StartX = PointArray[8][0];
			StartY = PointArray[0][1];
			EndX = PointArray[9][0];
			EndY = PointArray[8][1];

			CurrentX = StartX;
			while (CurrentX * dx < EndX * dx)
			{
				NextX = GetNextValue(CurrentX, dx, EndX);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �᭱
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion


					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentX = NextX;
			}

			// �Ĥ��q
			StartX = PointArray[11][0];
			StartY = PointArray[11][1];
			EndX = PointArray[10][0];
			EndY = PointArray[2][1];

			CurrentX = StartX;
			while (CurrentX * dx < EndX * dx)
			{
				NextX = GetNextValue(CurrentX, dx, EndX);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �᭱
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion


					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentX = NextX;
			}

			// �ĤC�q
			StartX = PointArray[9][0];
			StartY = PointArray[0][1];
			EndX = PointArray[1][0];
			EndY = PointArray[2][1];

			CurrentX = StartX;
			while (CurrentX * dx < EndX * dx)
			{
				NextX = GetNextValue(CurrentX, dx, EndX);
				CurrentY = StartY;

				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);

					#pragma region ��l��
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region �e��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �᭱
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region ����
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �k��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �W��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[0][2])));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region �U��
					vhandle.clear();

					if (dx < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2])));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, PointArray[0][2] + offsetZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, PointArray[0][2] + offsetZ)));
					}

					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion


					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentY = NextY;
				}
				CurrentX = NextX;
			}

			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;
			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
		}
		#pragma endregion 
		#pragma region ��L����
		// �γ����k���T����
		else if (InfoArray[i]->PartName.endsWith("/triangle"))
		{
			// ������
			InfoArray[i]->PartName = "Triangle/triangle";

			#pragma region 	�ư����~���p
			if (ModelsArray[i]->n_vertices() != 18)
			{
				cout << "Triangle �����~" << endl;
				continue;
			}
			#pragma endregion
			#pragma region �[�T���I x 2
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			int offsetArray[] = { 0,1,2,3,5,4 };
			for (int j = 0; j < 6; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region �}�l�n�[����
			QVector<MyMesh::VertexHandle>	vhandle;						// Vertex Handle
			
			// �[�W ���߹����쩳�U���I
			MyMesh::Point tempP = (PointArray[0] + PointArray[2]) / 2;
			PointArray.push_back(tempP);
			tempP = (PointArray[3] + PointArray[5]) / 2;
			PointArray.push_back(tempP);

			// �s�W info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;

			#pragma region �e�b�q
			// 6 �������A��n�����̶]
			float dz = (PointArray[0][2] - PointArray[6][2] > 0) ? 1 : -1;

			// �q�o�Ӷ}�l
			float CurrentZ;

			// �U�@�Ӫ� X, Z
			float NextZ;

			// �}�l����
			float StartZ = PointArray[6][2];

			// ��������
			float EndZ = PointArray[0][2];

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);

				float Prograss = (CurrentZ - StartZ) / (EndZ - StartZ);
				float NextPrograss = (NextZ - StartZ) / (EndZ - StartZ);

				float CurrentY = (PointArray[0][1] - PointArray[1][1]) * Prograss + PointArray[1][1];
				float NextY = (PointArray[0][1] - PointArray[1][1]) * NextPrograss + PointArray[1][1];

				MyMesh *tempMesh = new MyMesh;
				#pragma region �e��
				if (Prograss < 1)
				{
					vhandle.clear();

					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], NextZ)));
					tempMesh->add_face(vhandle.toStdVector());
				}
				#pragma endregion
				#pragma region �᭱
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region ����
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �k��
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �W
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �U
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion


				tempMesh->update_normals();
				SplitModelsArray.push_back(tempMesh);
				SplitCount++;

				CurrentZ = NextZ;
			}
			#pragma endregion
			#pragma region ��b�q
			// 6 �������A��n�����̶]
			dz = (PointArray[2][2] - PointArray[6][2] > 0) ? 1 : -1;

			// �q�o�Ӷ}�l
			CurrentZ;

			// �U�@�Ӫ� X, Z
			NextZ;

			// �}�l����
			StartZ = PointArray[6][2];

			// ��������
			EndZ = PointArray[2][2];

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);

				float Prograss = (CurrentZ - StartZ) / (EndZ - StartZ);
				float NextPrograss = (NextZ - StartZ) / (EndZ - StartZ);

				float CurrentY = (PointArray[2][1] - PointArray[1][1]) * Prograss + PointArray[1][1];
				float NextY = (PointArray[2][1] - PointArray[1][1]) * NextPrograss + PointArray[1][1];

				MyMesh *tempMesh = new MyMesh;
				#pragma region �e��
				if (Prograss < 1)
				{
					vhandle.clear();

					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], NextY, NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
					tempMesh->add_face(vhandle.toStdVector());
				}
				#pragma endregion
				#pragma region �᭱
				vhandle.clear();
				
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region ����
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �k��
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �W
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region �U
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion

				tempMesh->update_normals();
				SplitModelsArray.push_back(tempMesh);
				SplitCount++;

				CurrentZ = NextZ;
			}
			#pragma endregion
			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;

			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
		}
		#pragma endregion
	}

	for (int i = 0; i < SplitInfoArray.length(); i++)
	{
		cout << "Name\t\t=> " << SplitInfoArray[i]->PartName.toStdString() << endl;
		cout << "Start Index\t=> " << SplitInfoArray[i]->StartModelIndex << endl;
		cout << "Part Number\t=> " << SplitInfoArray[i]->PartNumber << endl;
		cout << "Part Size\t=> " << SplitInfoArray[i]->SplitCount << endl;
	}
	cout << "========== ���p���󧹦� ==========" << endl;
}
void InterLockClass::GenerateLock()
{
	#pragma region �����ͦۤv���񪺥d�g
	for (int i = 0; i < SplitInfoArray.length(); i++)
		for (int j = 0; j < SplitInfoArray[i]->LockDataInfo.size(); j++)
		{
			int indexPart = SplitInfoArray[i]->PartName.lastIndexOf('/');
			QString infoPartName = SplitInfoArray[i]->PartName.mid(0, indexPart);
			CountInfo info = SplitInfoArray[i]->LockDataInfo[j];

			#pragma region ���������
			if (infoPartName == "Base")
			{
				for (int z = 0; z < info.ZCount; z++)
					for (int x = 0; x < info.XCount; x++)
					{
						int startIndex = SplitInfoArray[i]->StartModelIndex;
						MyMesh *mesh = SplitModelsArray[x + z * info.XCount + startIndex];

						mesh->request_vertex_status();
						mesh->request_face_status();

						#pragma region ����Ҧ��|�ʨ쪺 FaceHandle ���s��̭�
						// ��X�n�R������
						QVector<MyMesh::Point> vArray[4];
						MyMesh::Point centerPos[4];

						MyMesh::FaceHandle *fArray = new MyMesh::FaceHandle[4];
						fArray[0] = FindFaceByDir(mesh, 'x', info.XDir, vArray[0], centerPos[0]);
						fArray[1] = FindFaceByDir(mesh, 'z', info.ZDir, vArray[1], centerPos[1]);
						fArray[2] = FindFaceByDir(mesh, 'x', -info.XDir, vArray[2], centerPos[2]);
						fArray[3] = FindFaceByDir(mesh, 'z', -info.ZDir, vArray[3], centerPos[3]);
						#pragma endregion

						// X �Y��
						if (x < info.XCount - 1)
						{
							MyMesh::FaceHandle fHandle = fArray[0];
							QVector<MyMesh::Point> tempVArray = vArray[0];
							MyMesh::Point tempCenterPos = centerPos[0];
							if (CountDistance(tempVArray[0], tempVArray[1]) >= MinSize && CountDistance(tempVArray[0], tempVArray[3]) >= MinSize)
							{
								// �R����
								mesh->delete_face(fHandle);

								// �[�W�d�g�A����X����
								MyMesh::Point *tempPoint = new MyMesh::Point[tempVArray.size()];
								for (int k = 0; k < tempVArray.size(); k++)
								{
									tempPoint[k] = (tempCenterPos + tempVArray[k]) / 2;
									if ((tempVArray[k][0] - tempCenterPos[0]) > 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0] - offset, tempPoint[k][1], tempPoint[k][2]);
									else if ((tempVArray[k][0] - tempCenterPos[0]) < 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0] + offset, tempPoint[k][1], tempPoint[k][2]);

									if ((tempVArray[k][1] - tempCenterPos[1]) > 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0], tempPoint[k][1] - offset, tempPoint[k][2]);
									else if ((tempVArray[k][1] - tempCenterPos[1]) < 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0], tempPoint[k][1] + offset, tempPoint[k][2]);

									if ((tempVArray[k][2] - tempCenterPos[2]) > 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0], tempPoint[k][1], tempPoint[k][2] - offset);
									else if ((tempVArray[k][2] - tempCenterPos[2]) < 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0], tempPoint[k][1], tempPoint[k][2] + offset);
								}
								#pragma region �Y��
								QVector<MyMesh::VertexHandle> tempHandle;
								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[0]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[1]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[2]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								mesh->add_face(tempHandle.toStdVector());


								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[3]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								mesh->add_face(tempHandle.toStdVector());

								// �Y�_����
								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(LockHeight - offset, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(LockHeight - offset, 0, 0)));
								mesh->add_face(tempHandle.toStdVector());
								#pragma endregion
							}
						}

						// Z �Y��
						if (z < info.ZCount - 1)
						{
							MyMesh::FaceHandle fHandle = fArray[1];
							QVector<MyMesh::Point> tempVArray = vArray[1];
							MyMesh::Point tempCenterPos = centerPos[1];
							if (CountDistance(tempVArray[0], tempVArray[1]) >= MinSize && CountDistance(tempVArray[0], tempVArray[3]) >= MinSize)
							{
								// �R����
								mesh->delete_face(fHandle);

								// �[�W�d�g�A����X����
								MyMesh::Point *tempPoint = new MyMesh::Point[tempVArray.size()];
								for (int k = 0; k < tempVArray.size(); k++)
								{
									tempPoint[k] = (tempCenterPos + tempVArray[k]) / 2;
									if ((tempVArray[k][0] - tempCenterPos[0]) > 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0] - offset, tempPoint[k][1], tempPoint[k][2]);
									else if ((tempVArray[k][0] - tempCenterPos[0]) < 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0] + offset, tempPoint[k][1], tempPoint[k][2]);

									if ((tempVArray[k][1] - tempCenterPos[1]) > 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0], tempPoint[k][1] - offset, tempPoint[k][2]);
									else if ((tempVArray[k][1] - tempCenterPos[1]) < 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0], tempPoint[k][1] + offset, tempPoint[k][2]);

									if ((tempVArray[k][2] - tempCenterPos[2]) > 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0], tempPoint[k][1], tempPoint[k][2] - offset);
									else if ((tempVArray[k][2] - tempCenterPos[2]) < 0)
										tempPoint[k] = MyMesh::Point(tempPoint[k][0], tempPoint[k][1], tempPoint[k][2] + offset);
								}
								#pragma region �Y��
								QVector<MyMesh::VertexHandle> tempHandle;
								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[0]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[1]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[2]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								mesh->add_face(tempHandle.toStdVector());


								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[3]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								mesh->add_face(tempHandle.toStdVector());

								// �Y�_����
								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(0, 0, LockHeight - offset)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(0, 0, LockHeight - offset)));
								mesh->add_face(tempHandle.toStdVector());
								#pragma endregion
							}
						}

						// X �W��
						if (x > 0)
						{
							MyMesh::FaceHandle fHandle = fArray[2];
							QVector<MyMesh::Point> tempVArray = vArray[2];
							MyMesh::Point tempCenterPos = centerPos[2];
							if (CountDistance(tempVArray[0], tempVArray[1]) >= MinSize && CountDistance(tempVArray[0], tempVArray[3]) >= MinSize)
							{
								// �R����
								mesh->delete_face(fHandle);

								// �[�W�d�g�A����X����
								MyMesh::Point *tempPoint = new MyMesh::Point[tempVArray.size()];
								for (int k = 0; k < tempVArray.size(); k++)
									tempPoint[k] = (tempCenterPos + tempVArray[k]) / 2;

								#pragma region �W��
								QVector<MyMesh::VertexHandle> tempHandle;
								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[0]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[1]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[2]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								mesh->add_face(tempHandle.toStdVector());


								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[3]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								mesh->add_face(tempHandle.toStdVector());

								// �Y�_����
								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(LockHeight, 0, 0)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(LockHeight, 0, 0)));
								mesh->add_face(tempHandle.toStdVector());
								#pragma endregion
							}
						}
						
						// Z �Y��
						if (z > 0)
						{
							MyMesh::FaceHandle fHandle = fArray[3];
							QVector<MyMesh::Point> tempVArray = vArray[3];
							MyMesh::Point tempCenterPos = centerPos[3];
							if (CountDistance(tempVArray[0], tempVArray[1]) >= MinSize && CountDistance(tempVArray[0], tempVArray[3]) >= MinSize)
							{
								// �R����
								mesh->delete_face(fHandle);

								// �[�W�d�g�A����X����
								MyMesh::Point *tempPoint = new MyMesh::Point[tempVArray.size()];
								for (int k = 0; k < tempVArray.size(); k++)
									tempPoint[k] = (tempCenterPos + tempVArray[k]) / 2;
								#pragma region �Y��
								QVector<MyMesh::VertexHandle> tempHandle;
								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[0]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[1]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[2]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								mesh->add_face(tempHandle.toStdVector());


								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempVArray[3]));
								tempHandle.push_back(mesh->add_vertex(tempVArray[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								mesh->add_face(tempHandle.toStdVector());

								// �Y�_����
								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[0]));
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3]));
								mesh->add_face(tempHandle.toStdVector());

								tempHandle.clear();
								tempHandle.push_back(mesh->add_vertex(tempPoint[0] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[1] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[2] - MyMesh::Point(0, 0, LockHeight)));
								tempHandle.push_back(mesh->add_vertex(tempPoint[3] - MyMesh::Point(0, 0, LockHeight)));
								mesh->add_face(tempHandle.toStdVector());
								#pragma endregion
							}
						}

						mesh->garbage_collection(true, false, true);
					}
			}
			#pragma endregion
		}
	#pragma endregion
}
void InterLockClass::SaveAllModel()
{
	cout << "========== �}�l�x�s ==========" << endl;
	if (!QDir(FilePathLocation + "/Output/").exists())
		QDir().mkdir(FilePathLocation + "/Output/");

	for (int i = 0; i < SplitInfoArray.length(); i++)
	{
		// �s�W Part ����Ƨ�
		int indexPart = SplitInfoArray[i]->PartName.lastIndexOf('/');
		QString infoPartName = SplitInfoArray[i]->PartName.mid(0, indexPart);

		// �ɮ׸��|
		QString tempFileLocation = FilePathLocation + "/Output/" + infoPartName;
		if (!QDir(tempFileLocation).exists())
			QDir().mkdir(tempFileLocation);

		for (int j = 0; j < SplitInfoArray[i]->SplitCount; j++)
		{
			int StartIndex = SplitInfoArray[i]->StartModelIndex;
			QString subLocation = "/model_part" + QString::number(SplitInfoArray[i]->PartNumber, 10);

			// �b�̭��Ыظ�Ƨ�
			if (!QDir(tempFileLocation + subLocation).exists())
				QDir().mkdir(tempFileLocation + subLocation);

			// �� obj �s�i��
			if (!OpenMesh::IO::write_mesh(*SplitModelsArray[j + StartIndex], 
				QString(tempFileLocation + subLocation + "/" + QString::number(j + 1) + outputFileEnd).toStdString().data()))
				cout << "�x�s���� => model_part" << SplitInfoArray[i]->PartNumber << "_" << (j + 1) << outputFileEnd.toStdString() << endl;
		}
	}
	cout << "========== �x�s���� ==========" << endl;
}

float InterLockClass::GetNextValue(float currentValue, float d, float max)
{
	float nextValue = currentValue + SplitSize * d;					// �U�@�ӭ�

	if (nextValue * d >= max * d && currentValue * d < max * d)
		nextValue = max;
	return nextValue;
}
uint InterLockClass::CountSize(float startIndex, float endIndex)
{
	return (abs(endIndex - startIndex) - 0.1) / SplitSize + 1;
}
MyMesh::FaceHandle InterLockClass::FindFaceByDir(MyMesh *mesh, char dir, int value, QVector<MyMesh::Point> &vArray, MyMesh::Point &center)
{
	// ��X�����I
	MyMesh::Point centerPos = MyMesh::Point(0, 0, 0);
	for (MyMesh::VertexIter v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		centerPos += mesh->point(v_it);
	centerPos /= mesh->n_vertices();

	// �ھ� dir ���V
	MyMesh::Point tempCenterPos;
	for (MyMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
	{
		tempCenterPos = MyMesh::Point(0, 0, 0);
		int FaceCount = 0;
		vArray.clear();

		for (MyMesh::FaceVertexIter fv_it = mesh->fv_iter(f_it); fv_it.is_valid(); ++fv_it)
		{
			tempCenterPos += mesh->point(fv_it);
			vArray.push_back(mesh->point(fv_it));
			FaceCount++;
		}
		tempCenterPos /= FaceCount;
		center = tempCenterPos;

		int compareValue;
		switch (dir)
		{
		case 'x':
		case 'X':
			compareValue = tempCenterPos[0] - centerPos[0];
			break;
		case 'y':
		case 'Y':
			compareValue = tempCenterPos[1] - centerPos[1];
			break;
		case 'z':
		case 'Z':
			compareValue = tempCenterPos[2] - centerPos[2];
			break;
		}

		if ((compareValue < 0 && value < 0) || (compareValue > 0 && value > 0))
			return f_it;
	}
	return mesh->faces_begin();
}

float InterLockClass::CountDistance(MyMesh::Point a, MyMesh::Point b)
{
	return sqrt((a[0] - b[0]) * (a[0] - b[0]) +
		(a[1] - b[1]) * (a[1] - b[1]) +
		(a[2] - b[2]) * (a[2] - b[2]));
}
