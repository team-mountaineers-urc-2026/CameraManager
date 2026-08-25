/**
 * @file startup.cpp
 * @author William Streck
 * @brief Initialization procedures for the cameras.
 * @version 0.1
 * @date 2024-10-24
 * 
 */

#include "startup.hpp"
#include "CameraManagerNode.hpp"

std::string get_device_file(int device_index) {
    if (device_index < 0) {
        return ""; // No device index specified
    }
    
    auto numstr = std::to_string(device_index);
    if (numstr.size() == 1) {
        numstr = "0" + numstr;
    }
    
    return "/dev/urc/cam/logitech_" + numstr;
}

cv::VideoCapture create_capture(settings set) {
    std::string dev_file;

    dev_file = get_device_file(set.device_index);

    if (set.device_index == -1 || dev_file.empty()) {
        // No device index specified, return an invalid capture
        RCLCPP_ERROR(camera_manager_node->get_logger(), "No device index specified for camera. How did you get here?????");
        return cv::VideoCapture("/dev/null"); // Should not be possible to get here
    }

    // Open the capture in MJPEG mode to avoid USB 2 bandwidth issues at 1080p
    auto cap = cv::VideoCapture(dev_file, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

    // mandatory
    cap.set(cv::CAP_PROP_FRAME_WIDTH, set.width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, set.height);
    cap.set(cv::CAP_PROP_FPS, set.fps);

    // optional
    cap.set(cv::CAP_PROP_BRIGHTNESS, set.brightness);
    cap.set(cv::CAP_PROP_CONTRAST, set.contrast);
    cap.set(cv::CAP_PROP_SATURATION, set.saturation);
    cap.set(cv::CAP_PROP_GAIN, set.gain);
    cap.set(cv::CAP_PROP_AUTO_WB, set.enable_auto_white_balance);

    return cap;
}