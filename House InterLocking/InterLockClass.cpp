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
		cout << "�I�� => " << tempMesh->n_vertices() << endl;

		tempMesh->request_vertex_status();
		tempMesh->request_edge_status();
		tempMesh->request_face_status();
		ModelsArray.push_back(tempMesh);

		ss >> info->PartName;
		ss >> tempInt;
		
		// �̭����d������T�A�Ȯɥ����|�Ψ�
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

		//qDebug() << "InterLockingFace length => " << info->InterLockingFace.length();
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

		if (InfoArray[i]->PartName.contains("Windows"))
		{
			cout << "�٨S��" << endl;
		}
		else if (InfoArray[i]->PartName.contains("roof"))
		{
			#pragma region 	�ư����~���p
			if (ModelsArray[i]->n_vertices() != 20)
			{
				cout << "Triangle �����~" << endl;
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
		else if (InfoArray[i]->PartName.contains(("Triangle")))
		{
			#pragma region 	�ư����~���p
			if (ModelsArray[i]->n_vertices() != 18)
			{
				cout << "Triangle �����~" << endl;
				return;
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
		else if (InfoArray[i]->PartName == "base" || InfoArray[i]->PartName.contains("Wall"))
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

void InterLockClass::SaveAllModel()
{
	cout << "========== �}�l�x�s ==========" << endl;
	if (!QDir(FilePathLocation + "/Output/").exists())
		QDir().mkdir(FilePathLocation + "/Output/");

	for (int i = 0; i < SplitInfoArray.length(); i++)
	{
		// �s�W Part ����Ƨ�
		QString infoPartName = SplitInfoArray[i]->PartName;
		if (!QDir(FilePathLocation + "/Output/" + infoPartName).exists())
			QDir().mkdir(FilePathLocation + "/Output/" + infoPartName);

		for (int j = 0; j < SplitInfoArray[i]->SplitCount; j++)
		{
			int StartIndex = SplitInfoArray[i]->StartModelIndex;

			// �� obj �s�i��
			if (!OpenMesh::IO::write_mesh(*SplitModelsArray[j + StartIndex], 
				QString(FilePathLocation + "Output/" + infoPartName + "/model_part" + QString::number(SplitInfoArray[i]->PartNumber, 10) + "_" + QString::number(j + 1) + outputFileEnd).toStdString().data()))
				cout << "�x�s���� => model_part" << SplitInfoArray[i]->PartNumber << "_" << (j + 1) << outputFileEnd.toStdString() << endl;
			else
				cout << "�x�s���\ => model_part" << SplitInfoArray[i]->PartNumber << "_" << (j + 1) << outputFileEnd.toStdString() << endl;
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