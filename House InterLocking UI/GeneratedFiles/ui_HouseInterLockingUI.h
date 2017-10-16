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
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include "openglwidget.h"

QT_BEGIN_NAMESPACE

class Ui_HouseInterLockingUIClass
{
public:
    QWidget *centralWidget;
    OpenGLWidget *openGLWidget;
    QComboBox *comboBox;
    QSlider *horizontalSlider;
    QSlider *horizontalSlider_2;
    QSlider *horizontalSlider_3;
    QSlider *horizontalSlider_4;
    QSlider *horizontalSlider_5;
    QSlider *horizontalSlider_6;

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
        comboBox = new QComboBox(centralWidget);
        comboBox->setObjectName(QStringLiteral("comboBox"));
        comboBox->setGeometry(QRect(1210, 60, 281, 31));
        horizontalSlider = new QSlider(centralWidget);
        horizontalSlider->setObjectName(QStringLiteral("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(1210, 200, 271, 22));
        horizontalSlider->setMinimum(1);
        horizontalSlider->setMaximum(50);
        horizontalSlider->setOrientation(Qt::Horizontal);
        horizontalSlider_2 = new QSlider(centralWidget);
        horizontalSlider_2->setObjectName(QStringLiteral("horizontalSlider_2"));
        horizontalSlider_2->setGeometry(QRect(1210, 270, 271, 22));
        horizontalSlider_2->setMinimum(1);
        horizontalSlider_2->setMaximum(50);
        horizontalSlider_2->setOrientation(Qt::Horizontal);
        horizontalSlider_3 = new QSlider(centralWidget);
        horizontalSlider_3->setObjectName(QStringLiteral("horizontalSlider_3"));
        horizontalSlider_3->setGeometry(QRect(1210, 330, 271, 22));
        horizontalSlider_3->setMinimum(0);
        horizontalSlider_3->setMaximum(100);
        horizontalSlider_3->setValue(0);
        horizontalSlider_3->setOrientation(Qt::Horizontal);
        horizontalSlider_4 = new QSlider(centralWidget);
        horizontalSlider_4->setObjectName(QStringLiteral("horizontalSlider_4"));
        horizontalSlider_4->setGeometry(QRect(1210, 400, 271, 22));
        horizontalSlider_4->setMinimum(0);
        horizontalSlider_4->setMaximum(100);
        horizontalSlider_4->setValue(50);
        horizontalSlider_4->setOrientation(Qt::Horizontal);
        horizontalSlider_5 = new QSlider(centralWidget);
        horizontalSlider_5->setObjectName(QStringLiteral("horizontalSlider_5"));
        horizontalSlider_5->setGeometry(QRect(1210, 450, 271, 22));
        horizontalSlider_5->setMinimum(0);
        horizontalSlider_5->setMaximum(100);
        horizontalSlider_5->setValue(0);
        horizontalSlider_5->setOrientation(Qt::Horizontal);
        horizontalSlider_6 = new QSlider(centralWidget);
        horizontalSlider_6->setObjectName(QStringLiteral("horizontalSlider_6"));
        horizontalSlider_6->setGeometry(QRect(1210, 520, 271, 22));
        horizontalSlider_6->setMinimum(0);
        horizontalSlider_6->setMaximum(100);
        horizontalSlider_6->setValue(50);
        horizontalSlider_6->setOrientation(Qt::Horizontal);
        HouseInterLockingUIClass->setCentralWidget(centralWidget);

        retranslateUi(HouseInterLockingUIClass);

        QMetaObject::connectSlotsByName(HouseInterLockingUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *HouseInterLockingUIClass)
    {
        HouseInterLockingUIClass->setWindowTitle(QApplication::translate("HouseInterLockingUIClass", "HouseInterLockingUI", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class HouseInterLockingUIClass: public Ui_HouseInterLockingUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HOUSEINTERLOCKINGUI_H
