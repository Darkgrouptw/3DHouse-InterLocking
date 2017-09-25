#include "HouseInterLockingUI.h"

HouseInterLockingUI::HouseInterLockingUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	connect(ui.loadButton,	SIGNAL(clicked()),					this, SLOT(LoadModel()));
	connect(ui.comboBox,	SIGNAL(currentIndexChanged(int)),	this, SLOT(ComboBoxChangeEvent(int)));

	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(SliderChangeEvent(int)));
	connect(ui.horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(SliderChangeEvent2(int)));
}

void HouseInterLockingUI::LoadInfoText(QStringList list, QString outputDir)
{
	//for(int i = 0; i < list.size(); i++)
	//	if (list[i].startsWith("model_part"))
	//	{
	//		// 加到 ComboBox 上面
	//		ui.comboBox->addItem(list[i]);
	//	}
}

void HouseInterLockingUI::LoadModel()
{
	/*QString fileName = QFileDialog::getOpenFileName(this, tr("Open info.txt"), "../Test Sets/Demo/", tr("Info file(*.txt)"));

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
	}*/
}

void HouseInterLockingUI::ComboBoxChangeEvent(int index)
{
	/*IsChanging = true;
	cout << "Change =>" << index << endl;
	
	if (ui.openGLWidget->Demo_ColTotal[index] != 0)
		ui.horizontalSlider_2->setEnabled(true);
	else
		ui.horizontalSlider_2->setEnabled(false);

	ui.horizontalSlider->setValue(ui.openGLWidget->Demo_RowTotal[index]);
	ui.horizontalSlider_2->setValue(ui.openGLWidget->Demo_ColTotal[index]);

	ui.openGLWidget->chooseIndex = index;
	ui.openGLWidget->update();

	IsChanging = false;*/
}

void HouseInterLockingUI::SliderChangeEvent(int value)
{
	/*if (!IsChanging)
	{
		ui.openGLWidget->Demo_RowTotal[ui.openGLWidget->chooseIndex] = value;
		ui.openGLWidget->update();
	}*/
}
void HouseInterLockingUI::SliderChangeEvent2(int value)
{
	/*if (!IsChanging)
	{
		ui.openGLWidget->Demo_ColTotal[ui.openGLWidget->chooseIndex] = value;
		ui.openGLWidget->update();
	}*/
}