#include "HouseInterLockingUI.h"

HouseInterLockingUI::HouseInterLockingUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	connect(ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ComboBoxChangeEvent(int)));

	// 大小變
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(SliderChangeEvent(int)));
	connect(ui.horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(SliderChangeEvent(int)));

	// 比例變
	connect(ui.horizontalSlider_3, SIGNAL(valueChanged(int)), this, SLOT(SliderChangeEvent(int)));
	connect(ui.horizontalSlider_4, SIGNAL(valueChanged(int)), this, SLOT(SliderChangeEvent(int)));

	ui.openGLWidget->comboBox = ui.comboBox;
}

void HouseInterLockingUI::ComboBoxChangeEvent(int index)
{
	cout << "選單編號 => " << index << endl;

	IsChanging = true;
	selectIndex = index;

	NodeInfo *info = ui.openGLWidget->queueInfo[index];
	switch (index)
	{
	case 0:
		// 地板
		SetSliderFunction(true, true, false, false);

		ui.horizontalSlider->setValue(info->nParams.XLength);
		ui.horizontalSlider_2->setValue(info->nParams.ZLength);
		break;
	case 1:
		// 單窗
		SetSliderFunction(false, false, true, true);

		ui.horizontalSlider_3->setValue(info->singleWindowParams.RatioWidth * 10);
		ui.horizontalSlider_4->setValue(info->singleWindowParams.RatioHeight * 10);
	case 2:
		// 門
		SetSliderFunction(false, false, true, false);

		ui.horizontalSlider_3->setValue(info->doorParams.ratio * 10);
		break;
	default:
		break;
	}
	IsChanging = false;
}
void HouseInterLockingUI::SetSliderFunction(bool boolean1, bool boolean2, bool boolean3, bool boolean4)
{
	ui.horizontalSlider->setEnabled(boolean1);
	ui.horizontalSlider_2->setEnabled(boolean2);
	ui.horizontalSlider_3->setEnabled(boolean3);
	ui.horizontalSlider_4->setEnabled(boolean4);
}

void HouseInterLockingUI::SliderChangeEvent(int value)
{
	if (!IsChanging)
	{
		ui.openGLWidget->UpdateByParams(selectIndex, 
			ui.horizontalSlider->value(),
			ui.horizontalSlider_2->value(),
			(float)ui.horizontalSlider_3->value() / ui.horizontalSlider_3->maximum(),
			(float)ui.horizontalSlider_4->value() / ui.horizontalSlider_4->maximum());

		ui.openGLWidget->update();
	}
}