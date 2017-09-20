#include "HouseInterLockingUI.h"

HouseInterLockingUI::HouseInterLockingUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	connect(ui.loadButton,	SIGNAL(clicked()),					this, SLOT(LoadModel()));
	connect(ui.comboBox,	SIGNAL(currentIndexChanged(int)),	this, SLOT(ComboBoxChangeEvent(int)));
}

void HouseInterLockingUI::LoadInfoText(QStringList list, QString outputDir)
{
	// 模型刪除
	ui.openGLWidget->pointDataArray.clear();

	for(int i = 0; i < list.size(); i++)
		if (list[i].startsWith("model_part"))
		{
			QStringList splitList = list[i].split('/');
			QString PartName = splitList[splitList.count() - 1].mid(0, splitList[splitList.count() - 1].size() - 1);		//好像會多一個 '/r'
			QString FolderName;

			#pragma region 找出是哪一個部分
			if (PartName == "gable" || PartName == "cross_gable")
				FolderName = "/Roof/";
			else if (PartName == "basic")
				FolderName = "/Base/";
			else if (PartName == "no_window" || PartName == "door_entry" || PartName == "single_window" || PartName == "multi_window")
				FolderName = "/Wall/";
			else if (PartName == "triangle")
				FolderName = "/Triangle/";
			#pragma endregion
			#pragma region 加入陣列裡
			QStringList folderList = QDir(outputDir + FolderName).entryList();
			for (int j = 2; j < folderList.size(); j++)
			{
				int index = folderList[j].mid(10, folderList[j].size() - 10).toInt();
				if (ui.openGLWidget->pointDataArray[index].size() != 0)
					break;

				QStringList fileList = QDir(outputDir + FolderName + folderList[j]).entryList();
				cout << (outputDir + FolderName + folderList[j]).toStdString() << endl;
				
				// 假設沒有加入過
				QVector<QVector<QVector<QVector3D>>> &vArray = ui.openGLWidget->pointDataArray[index];
				for (int k = 2; k < fileList.size(); k++)
					vArray.push_back(TransferAllMeshToQVector3D(outputDir + FolderName + folderList[j] + "/" + fileList[k]));
			}
			#pragma endregion
			#pragma region 把項目加到 Combox 裡面
			// 加到 ComboBox 上面
			ui.comboBox->addItem(list[i]);
			#pragma endregion
		}
		else if (i == 0)
		{
			int number = list[i].toInt();
			for (int i = 0; i < number; i++)
			{
				QVector<QVector<QVector<QVector3D>>> tempArray;
				ui.openGLWidget->pointDataArray.push_back(tempArray);
			}
		}
}

QVector<QVector<QVector3D>> HouseInterLockingUI::TransferAllMeshToQVector3D(QString folderLocation)
{
	MyMesh tempMesh;
	OpenMesh::IO::read_mesh(tempMesh, folderLocation.toStdString());

	QVector<QVector<QVector3D>> VertexArray;
	for (MyMesh::FaceIter f_it = tempMesh.faces_begin(); f_it != tempMesh.faces_end(); f_it++)
	{
		QVector<QVector3D> FaceVertex;
		for (MyMesh::FaceVertexIter fv_it = tempMesh.fv_iter(f_it); fv_it.is_valid(); fv_it++)
		{
			float* points = tempMesh.point(fv_it).data();
			FaceVertex.push_back(QVector3D(points[0], points[1], points[2]));
		}
		VertexArray.push_back(FaceVertex);
	}
	return VertexArray;
}

void HouseInterLockingUI::LoadModel()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open info.txt"), "../Test Sets/Demo/", tr("Info file(*.txt)"));
	
	if (fileName != "")
	{
		QFile file(fileName);
		file.open(QIODevice::ReadOnly);
		QTextStream ss(&file);
		QStringList slist = ss.readAll().split("\n");

		QString outputDir = fileName;
		outputDir.replace("info.txt", "Output");
		if (QDir(outputDir).exists())
			LoadInfoText(slist, outputDir);
		
		cout << slist.count() << " " << slist[0].toStdString() << endl;
		cout << outputDir.toStdString() << endl;
		cout << fileName.toStdString() << endl;
	}
}

void HouseInterLockingUI::ComboBoxChangeEvent(int index)
{
	cout << "Change =>" << index << endl;
	ui.openGLWidget->chooseIndex = index;
	ui.openGLWidget->update();
}
