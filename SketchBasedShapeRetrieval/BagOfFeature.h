#pragma once

#include <string>
#include "Camera.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <vector>

class BagOfFeature {
public:
	std::string filepath;
	Camera camera;
	std::vector<cv::Mat> features;
	cv::Mat histogram;

public:
	BagOfFeature(const cv::Mat& image, const std::string& filepath, const Camera& camera, float sigma, float lmbd, int patch_width, int patch_height);

	void computeHistogram(const std::vector<cv::Mat>& visualWords);
	void extractFeatures(const cv::Mat& image, float sigma, float lmbd, int patch_width , int patch_height);
	void findSimilarModels(const std::vector<BagOfFeature>& bofs, std::vector<int>& results, int n);
	static float similarity(const cv::Mat& h1, const cv::Mat& h2);
};

