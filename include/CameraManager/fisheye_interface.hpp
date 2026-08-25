/**
 * @file fisheye_interface.hpp
 * @author William Streck
 * @brief Fisheye interface header for the Reolink camera.
 * @version 0.1
 * @date 2025-05-08
 * 
 */

 #ifndef FISHEYE_INTERFACE_HPP
 #define FISHEYE_INTERFACE_HPP

 #include "camera_thread.hpp" // for externs

 using namespace std;

 #define FISHEYE_URL "rtsp://admin:daedalus@192.168.1.10/Preview_01_sub" ///< Fisheye sub stream URL

 /**
 * @brief Fisheye camera thread function.
 * Pulls the rtsp stream locally and recompresses it to a local stream.
 * 
 * @param parsed Camera information map.
 * @param tmap_index Numeric index in thread map. Of minimal significance.
 */
void fisheye_cam_thread(map<string, int> parsed, int tmap_index);

 #endif