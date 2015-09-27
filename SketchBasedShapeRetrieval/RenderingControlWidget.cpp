#include "RenderingControlWidget.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "GLWidget3D.h"

RenderingControlWidget::RenderingControlWidget(MainWindow* mainWin) : QDockWidget("Rendering Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);

	ui.horizontalSliderDepthSensitivity->setMinimum(0);
	ui.horizontalSliderDepthSensitivity->setMaximum(30000);
	ui.horizontalSliderDepthSensitivity->setValue(6000);
	ui.horizontalSliderNormalSensitivity->setMinimum(0);
	ui.horizontalSliderNormalSensitivity->setMaximum(200);
	ui.horizontalSliderNormalSensitivity->setValue(100);
	ui.checkBoxThreshold->setChecked(true);
	ui.lineEditThreshold->setText("0.25");

	// terrain
	connect(ui.horizontalSliderDepthSensitivity, SIGNAL(valueChanged(int)), this, SLOT(updateRendering(int)));
	connect(ui.horizontalSliderNormalSensitivity, SIGNAL(valueChanged(int)), this, SLOT(updateRendering(int)));
	connect(ui.checkBoxThreshold, SIGNAL(clicked()), this, SLOT(updateRendering()));

	updateRendering(0);
	//hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void RenderingControlWidget::updateRendering() {
	ui.lineEditDepthSensitivity->setText(QString::number(ui.horizontalSliderDepthSensitivity->value()));
	ui.lineEditNormalSensitivity->setText(QString::number((float)ui.horizontalSliderNormalSensitivity->value() / 100.0f));
	mainWin->glWidget->depthSensitivity = ui.horizontalSliderDepthSensitivity->value();
	mainWin->glWidget->normalSensitivity = (float)ui.horizontalSliderNormalSensitivity->value() / 100.0f;
	mainWin->glWidget->useThreshold = ui.checkBoxThreshold->isChecked();
	mainWin->glWidget->threshold = ui.lineEditThreshold->text().toFloat();
	mainWin->glWidget->updateGL();
}

void RenderingControlWidget::updateRendering(int) {
	updateRendering();
}