/**
 * @file realsense_interface.hpp
 * @author William Streck
 * @brief Interface functions for the realsense package.
 * @version 0.1
 * @date 2025-02-22
 * 
 */

#ifndef REALSENSE_INTERFACE_HPP
#define REALSENSE_INTERFACE_HPP

#include "camera_thread.hpp" // for externs

using namespace std;

#define REALSENSE_NATIVE_FPS 30 ///< FPS from realsense camera
#define REALSENSE_DEFAULT_WIDTH 848 ///< Realsense node cam width
#define REALSENSE_DEFAULT_HEIGHT 480 ///< Realsense node cam height

/**
 * @brief Intel RealSense camera thread function.
 * Directly interfaces with realsense-ros package.
 * 
 * @param parsed Camera information map.
 * @param tmap_index Numeric index in thread map. Of minimal significance.
 */
void realsense_cam_thread(map<string, int> parsed, int tmap_index);

#endif
