/********************************************************************************
** Form generated from reading UI file 'RenderingControlWidget.ui'
**
** Created: Mon Sep 28 15:15:51 2015
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RENDERINGCONTROLWIDGET_H
#define UI_RENDERINGCONTROLWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSlider>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RenderingControlWidget
{
public:
    QWidget *dockWidgetContents;
    QSlider *horizontalSliderDepthSensitivity;
    QLabel *label;
    QLabel *label_2;
    QSlider *horizontalSliderNormalSensitivity;
    QLineEdit *lineEditDepthSensitivity;
    QLineEdit *lineEditNormalSensitivity;
    QCheckBox *checkBoxThreshold;
    QLineEdit *lineEditThreshold;

    void setupUi(QDockWidget *RenderingControlWidget)
    {
        if (RenderingControlWidget->objectName().isEmpty())
            RenderingControlWidget->setObjectName(QString::fromUtf8("RenderingControlWidget"));
        RenderingControlWidget->resize(198, 621);
        RenderingControlWidget->setMinimumSize(QSize(190, 240));
        RenderingControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        horizontalSliderDepthSensitivity = new QSlider(dockWidgetContents);
        horizontalSliderDepthSensitivity->setObjectName(QString::fromUtf8("horizontalSliderDepthSensitivity"));
        horizontalSliderDepthSensitivity->setGeometry(QRect(20, 40, 160, 19));
        horizontalSliderDepthSensitivity->setOrientation(Qt::Horizontal);
        label = new QLabel(dockWidgetContents);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 10, 91, 16));
        label_2 = new QLabel(dockWidgetContents);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 80, 91, 16));
        horizontalSliderNormalSensitivity = new QSlider(dockWidgetContents);
        horizontalSliderNormalSensitivity->setObjectName(QString::fromUtf8("horizontalSliderNormalSensitivity"));
        horizontalSliderNormalSensitivity->setGeometry(QRect(20, 100, 160, 19));
        horizontalSliderNormalSensitivity->setOrientation(Qt::Horizontal);
        lineEditDepthSensitivity = new QLineEdit(dockWidgetContents);
        lineEditDepthSensitivity->setObjectName(QString::fromUtf8("lineEditDepthSensitivity"));
        lineEditDepthSensitivity->setGeometry(QRect(112, 10, 71, 20));
        lineEditNormalSensitivity = new QLineEdit(dockWidgetContents);
        lineEditNormalSensitivity->setObjectName(QString::fromUtf8("lineEditNormalSensitivity"));
        lineEditNormalSensitivity->setGeometry(QRect(120, 80, 61, 20));
        checkBoxThreshold = new QCheckBox(dockWidgetContents);
        checkBoxThreshold->setObjectName(QString::fromUtf8("checkBoxThreshold"));
        checkBoxThreshold->setGeometry(QRect(20, 150, 70, 17));
        lineEditThreshold = new QLineEdit(dockWidgetContents);
        lineEditThreshold->setObjectName(QString::fromUtf8("lineEditThreshold"));
        lineEditThreshold->setGeometry(QRect(100, 150, 81, 20));
        RenderingControlWidget->setWidget(dockWidgetContents);

        retranslateUi(RenderingControlWidget);

        QMetaObject::connectSlotsByName(RenderingControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *RenderingControlWidget)
    {
        label->setText(QApplication::translate("RenderingControlWidget", "Depth Sensitivity:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("RenderingControlWidget", "Normal Sensitivity:", 0, QApplication::UnicodeUTF8));
        checkBoxThreshold->setText(QApplication::translate("RenderingControlWidget", "threshold", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(RenderingControlWidget);
    } // retranslateUi

};

namespace Ui {
    class RenderingControlWidget: public Ui_RenderingControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RENDERINGCONTROLWIDGET_H
