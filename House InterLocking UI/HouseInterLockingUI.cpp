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
	connect(ui.horizontalSlider_5, SIGNAL(valueChanged(int)), this, SLOT(SliderChangeEvent(int)));
	connect(ui.horizontalSlider_6, SIGNAL(valueChanged(int)), this, SLOT(SliderChangeEvent(int)));

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
		SetSliderFunction(true, true, false, false, false, false);

		ui.horizontalSlider->setValue(info->nParams.XLength);
		ui.horizontalSlider_2->setValue(info->nParams.ZLength);
		break;
	case 1:
		// 單窗
		SetSliderFunction(false, false, true, true, false, false);

		ui.horizontalSlider_3->setValue(info->singleWindowParams.RatioWidth * 100);
		ui.horizontalSlider_4->setValue(info->singleWindowParams.RatioHeight * 100);
		break;
	case 2:
		// 門
		SetSliderFunction(false, false, true, false, false, false);

		ui.horizontalSlider_3->setValue(info->doorParams.ratio * 100);
		break;
	case 3:
		// 多窗
		SetSliderFunction(true, true, true, true, true, true);

		ui.horizontalSlider->setValue(info->multiWindowParams.windowA.WindowWidth);
		ui.horizontalSlider_2->setValue(info->multiWindowParams.windowA.WindowHeight);
		ui.horizontalSlider_3->setValue(info->multiWindowParams.windowA.RatioWidth * 100);
		ui.horizontalSlider_4->setValue(info->multiWindowParams.windowA.RatioHeight * 100);
		ui.horizontalSlider_5->setValue(info->multiWindowParams.windowB.RatioWidth * 100);
		ui.horizontalSlider_6->setValue(info->multiWindowParams.windowB.RatioHeight * 100);
		break;
	case 4:
	case 5:
		SetSliderFunction(false, false, false, false, false, false);
		break;
	case 6:
		SetSliderFunction(true, true, true, false, false, false);

		ui.horizontalSlider->setValue(info->gableParams.ConvexWidth);
		ui.horizontalSlider_2->setValue(info->gableParams.ConvexHeight);
		ui.horizontalSlider_3->setValue(info->gableParams.ratio * 100);
	default:
		break;
	}
	IsChanging = false;
}
void HouseInterLockingUI::SetSliderFunction(bool boolean1, bool boolean2, bool boolean3, bool boolean4, bool boolean5, bool boolean6)
{
	ui.horizontalSlider->setEnabled(boolean1);
	ui.horizontalSlider_2->setEnabled(boolean2);
	ui.horizontalSlider_3->setEnabled(boolean3);
	ui.horizontalSlider_4->setEnabled(boolean4);
	ui.horizontalSlider_5->setEnabled(boolean5);
	ui.horizontalSlider_6->setEnabled(boolean6);
}

void HouseInterLockingUI::SliderChangeEvent(int value)
{
	if (!IsChanging)
	{
		ui.openGLWidget->UpdateByParams(selectIndex,
			ui.horizontalSlider->value(),
			ui.horizontalSlider_2->value(),
			(float)ui.horizontalSlider_3->value() / ui.horizontalSlider_3->maximum(),
			(float)ui.horizontalSlider_4->value() / ui.horizontalSlider_4->maximum(),
			(float)ui.horizontalSlider_5->value() / ui.horizontalSlider_5->maximum(),
			(float)ui.horizontalSlider_6->value() / ui.horizontalSlider_6->maximum());
		ui.openGLWidget->update();
	}
}