#ifndef PROCESSING_H
#define	PROCESSING_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#define MAX_OPEN_FAILURES 500
#define MAX_CALIBRATE_FAILURES 3

/**
 * @brief Processes the image and update the robots' coordinates.
 * @param img
 */
void processImage(cv::Mat &img);

/**
 * Capture and run image processing until tracking mode is on.
 */
void captureLoop();

/**
 * Capture one frame and stores it in parameter.
 * @param img
 * @return 
 */
bool captureFrame(cv::Mat& img);

/**
 * Used to sort a vector of Moments.
 * @param m1
 * @param m2
 * @return 
 */
bool sortMoments(const cv::Moments& m1, const cv::Moments m2);

/**
 * Find contours and computes moments.
 * @param vec_moments
 * @param img
 */
void extractMoments(std::vector<cv::Moments>& vec_moments, cv::Mat& img);

/**
 * Compute moments of passed contours.
 * @param vec_moments
 * @param vectors
 * @return 
 */
std::vector<cv::Moments>& computeMoments(std::vector<cv::Moments>& vec_moments, std::vector<std::vector<cv::Point>> vectors);

/**
 * Prepares image for coordinates extraction.
 * @param imgOriginal
 * @return The thresholded image.
 * Change image to HSV and apply threshold.
 */
void prepareImage(cv::Mat& imgOriginal);


/**
 * Calibrates tracking system and sets Robots' ratio.
 * @param value
 * @parem nbRobots
 * @return 
 */
std::string calibrate(double value, int nbRobots);

cv::Point extractCoordinates(cv::Moments& m);
#endif	/* PROCESSING_H */

