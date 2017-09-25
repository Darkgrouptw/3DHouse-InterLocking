/********************************************************************************
** Form generated from reading UI file 'HouseInterLockingUI.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HOUSEINTERLOCKINGUI_H
#define UI_HOUSEINTERLOCKINGUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include "openglwidget.h"

QT_BEGIN_NAMESPACE

class Ui_HouseInterLockingUIClass
{
public:
    QWidget *centralWidget;
    OpenGLWidget *openGLWidget;
    QPushButton *loadButton;
    QComboBox *comboBox;
    QSlider *horizontalSlider;
    QSlider *horizontalSlider_2;

    void setupUi(QMainWindow *HouseInterLockingUIClass)
    {
        if (HouseInterLockingUIClass->objectName().isEmpty())
            HouseInterLockingUIClass->setObjectName(QStringLiteral("HouseInterLockingUIClass"));
        HouseInterLockingUIClass->resize(1500, 600);
        centralWidget = new QWidget(HouseInterLockingUIClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        openGLWidget = new OpenGLWidget(centralWidget);
        openGLWidget->setObjectName(QStringLiteral("openGLWidget"));
        openGLWidget->setGeometry(QRect(0, 0, 1200, 600));
        loadButton = new QPushButton(centralWidget);
        loadButton->setObjectName(QStringLiteral("loadButton"));
        loadButton->setGeometry(QRect(1220, 220, 271, 50));
        comboBox = new QComboBox(centralWidget);
        comboBox->setObjectName(QStringLiteral("comboBox"));
        comboBox->setGeometry(QRect(1210, 60, 281, 31));
        horizontalSlider = new QSlider(centralWidget);
        horizontalSlider->setObjectName(QStringLiteral("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(1210, 400, 271, 22));
        horizontalSlider->setMinimum(1);
        horizontalSlider->setMaximum(50);
        horizontalSlider->setOrientation(Qt::Horizontal);
        horizontalSlider_2 = new QSlider(centralWidget);
        horizontalSlider_2->setObjectName(QStringLiteral("horizontalSlider_2"));
        horizontalSlider_2->setGeometry(QRect(1210, 470, 271, 22));
        horizontalSlider_2->setMinimum(1);
        horizontalSlider_2->setMaximum(50);
        horizontalSlider_2->setOrientation(Qt::Horizontal);
        HouseInterLockingUIClass->setCentralWidget(centralWidget);

        retranslateUi(HouseInterLockingUIClass);

        QMetaObject::connectSlotsByName(HouseInterLockingUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *HouseInterLockingUIClass)
    {
        HouseInterLockingUIClass->setWindowTitle(QApplication::translate("HouseInterLockingUIClass", "HouseInterLockingUI", Q_NULLPTR));
        loadButton->setText(QApplication::translate("HouseInterLockingUIClass", "Load Model", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class HouseInterLockingUIClass: public Ui_HouseInterLockingUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HOUSEINTERLOCKINGUI_H
