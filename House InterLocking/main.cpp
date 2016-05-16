#include "main.h"

void ReadFile(char* inputFile)
{
	QString tempStr = QString(inputFile);
	QFile file(tempStr);
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "Input File can't open !!";
		return;
	}
	QTextStream ss(&file);
	int tempInt,totalNumber;
	float tempFloat, tempX, tempY, tempZ, tempType;

	QString InputFileStr(inputFile);
	QStringList FilePathList = InputFileStr.split("/");
	InputFileStr = "";
	for (int i = 0; i < FilePathList.length() - 1; i++)
		InputFileStr += FilePathList[i] + "/";

	ss >> totalNumber;
	qDebug() << "Total Number => " << totalNumber;
	for (int i = 0; i < totalNumber; i++)
	{
		MyMesh *tempMesh = new MyMesh();
		InterLockingInfo *info = new InterLockingInfo();
		ss >> tempStr;
		OpenMesh::IO::read_mesh(*tempMesh, (InputFileStr + tempStr).toLocal8Bit().data());
		tempStr.replace("model_part", "");
		info->PartNumber = tempStr.toInt();
		qDebug() << "Number of part => " << info->PartNumber;

		tempMesh->request_vertex_status();
		tempMesh->request_edge_status();
		tempMesh->request_face_status();
		Model.push_back(tempMesh);

		ss >> info->PartName;
		ss >> tempInt;
		for (int j = 0; j < tempInt / 2; j++)
			for (int k = 0; k < 6; k++)
			{
				ss >> tempX >> tempY >> tempZ >> tempY >> tempType;
				switch (k)
				{
				case 0:
					info->InterLockType = tempType;
					info->InterLockingFace.push_back(MyMesh::Point(tempX, tempY, tempZ));
					break;
				case 1:
				case 2:
				case 5:
					info->InterLockingFace.push_back(MyMesh::Point(tempX, tempY, tempZ));
					break;
				}
			}
	}
	file.close();
}



void AddInterLocking()
{

}

int main(int argc, char *argv[])
{
	if (argc == 2)
	{
		ReadFile(argv[1]);
		AddInterLocking();
	}
	else
		qDebug() << "Please add file path !!";
	return 0;
}