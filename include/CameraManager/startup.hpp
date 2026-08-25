/**
 * @file startup.hpp
 * @author William Streck
 * @brief Initialization procedures for the cameras.
 * @version 0.1
 * @date 2024-10-24
 * 
 */

#ifndef STARTUP_HPP
#define STARTUP_HPP

#include "settings.hpp"
#include <opencv4/opencv2/opencv.hpp>

/**
 * @brief Configures the camera with the given settings.
 * 
 * @param set settings object with the desired camera settings.
 * @return cv::VideoCapture the configured camera.
 */
cv::VideoCapture create_capture(settings set);

/**
 * @brief Gets the device file path for the camera based on the device index.
 * 
 * @param device_index Index of the camera device.
 * @return std::string Path to the camera device file (symlink).
 */
std::string get_device_file(int device_index);

#endif
