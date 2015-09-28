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
	int vocabulary_size;
	float sigma;
	float lmbd;
	int patch_width;
	int patch_height;
	bool useTfidf;

	std::vector<BagOfFeature> features;		// 各要素は、各画像の特徴量
	std::vector<cv::Mat> centroids;			// featureのクラスタ中心
	std::vector<float> frequencies;			// 各クラスタの頻度


public:
	ShapeMatching(GLWidget3D* glWidget3D, int width, int height, int vocabulary_size, float sigma, float lmbd, int patch_width, int patch_height, bool useTfidf);
	~ShapeMatching();

	void learn(const std::string& traindata_dir, int pitch_angle_num, int yaw_angle_num);
	void extractFeatures(const std::string& filename, int pitch_angle_num, int yaw_angle_num, std::vector<BagOfFeature>& features);
	float test(const std::string& testdata_dir, int topN);
	void kmeans(const std::vector<BagOfFeature>& features, std::vector<cv::Mat>& centroids, std::vector<float>& frequencies);
};

