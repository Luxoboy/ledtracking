#ifndef PROCESSING_H
#define	PROCESSING_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

/**
 * @brief Processes the image and extracts the coordinates.
 * @param img
 */
void processImage(cv::Mat &img);

/**
 * 
 */
void captureLoop();

/**
 * Used to sort a vector of Moments.
 * @param m1
 * @param m2
 * @return 
 */
bool sortMoments(const cv::Moments& m1, const cv::Moments m2);

std::vector<cv::Moments>& computeMoments(std::vector<cv::Moments>& vec_moments, std::vector<std::vector<cv::Point>> vectors);
#endif	/* PROCESSING_H */

