#pragma once

#include <glew.h>
#include "Shader.h"
#include "Vertex.h"
#include <QGLWidget>
#include <QMouseEvent>
#include "Camera.h"
#include "ShadowMapping.h"
#include "RenderManager.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "BagOfFeature.h"
#include "SketchyRenderingBuffer.h"

class MainWindow;

class GLWidget3D : public QGLWidget {
public:
	GLWidget3D(QWidget *parent = 0);

	void drawScene(int drawMode);
	void loadObject(const std::string& filename);
	void galifTest();
	void gaborFilterTest();
	void extractFeatures(const std::string& filename, int pitch_angle_num, int yaw_angle_num, std::vector<BagOfFeature>& features);
	void renderImage(cv::Mat& image);
	//void gaborFilter(const cv::Mat& src_img, std::vector<cv::Mat>& dst_imgs);
	//void computeGALIF(const std::vector<cv::Mat>& filteredImages, BagOfFeature& bof);
	void kmeans(const std::vector<BagOfFeature>& features, std::vector<cv::Mat>& centroids);

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();    
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);

public:
	static enum { RENDERING_MODE_REGULAR = 0, RENDERING_MODE_LINE };

public:
	Camera camera;
	glm::vec3 light_dir;
	glm::mat4 light_mvpMatrix;

	RenderManager renderManager;

	std::vector<Vertex> vertices;
	SketchyRenderingBuffer rb;

	int renderingMode;
	float depthSensitivity;
	float normalSensitivity;
	bool useThreshold;
	float threshold;
};

