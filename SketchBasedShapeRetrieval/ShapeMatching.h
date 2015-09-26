#pragma once

#include <string>
#include <vector>
#include "BagOfFeature.h"

class GLWidget3D;

class ShapeMatching {
public:
	std::string traindata_dir;
	std::string testdata_dir;
	int pitch_angle_num;
	int yaw_angle_num;
	GLWidget3D* glWidget3D;

	int width;
	int height;
	float sigma;
	float lmbd;
	int patch_width;
	int patch_height;

	std::vector<BagOfFeature> features;		// 各要素は、各画像の特徴量
	std::vector<cv::Mat> centroids;			// featureをクラスタ中心


public:
	ShapeMatching(GLWidget3D* glWidget3D, int width, int height, float sigma, float lmbd, int patch_width, int patch_height);
	~ShapeMatching();

	void learn(const std::string& traindata_dir, int pitch_angle_num, int yaw_angle_num);
	void extractFeatures(const std::string& filename, int pitch_angle_num, int yaw_angle_num, std::vector<BagOfFeature>& features);
	void test(const std::string& testdata_dir);
	void kmeans(const std::vector<BagOfFeature>& features, std::vector<cv::Mat>& centroids);
};

