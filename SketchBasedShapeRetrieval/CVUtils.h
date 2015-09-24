#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <vector>

namespace cvutils {

double mat_get_value(const cv::Mat& m, int r, int c);
void mat_set_value(cv::Mat& m, int r, int c, double val);
void mat_add_value(cv::Mat& m, int r, int c, double val);
double mat_sum(const cv::Mat& m);
double mat_dot(const cv::Mat& m1, const cv::Mat& m2);
void mat_save(char* filename, const cv::Mat& mat, bool normalize);
cv::Rect computeBoundingBoxFromImage(const cv::Mat& img);

}
