#pragma once

#include <QDockWidget>
#include "ui_RenderingControlWidget.h"

class MainWindow;

class RenderingControlWidget : public QDockWidget {
Q_OBJECT

private:
	
	MainWindow* mainWin;

public:
	Ui::RenderingControlWidget ui;
	RenderingControlWidget(MainWindow* mainWin);

	//void setRoadVertex(RoadVertexDesc vertexDesc, RoadVertexPtr selectedVertex);
	//void setRoadEdge(RoadEdgePtr selectedEdge);

public slots:
	void updateRendering();
	void updateRendering(int);

};

