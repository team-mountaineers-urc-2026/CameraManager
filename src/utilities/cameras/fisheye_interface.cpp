/**
 * @file fisheye_interface.cpp
 * @author William Streck
 * @brief Fisheye interface functions for the Camera Manager.
 * @version 0.1
 * @date 2025-02-22
 * 
 */

#include "fisheye_interface.hpp"
#include "CameraManagerNode.hpp"

void fisheye_cam_thread(map<string, int> parsed, int tmap_index) {
    if (parsed[INDEX_ID] != FISHEYE_ID) {
        threads_end.push_back(tmap_index);
        return;
    }

    // Get the important stuff, we can just grab straight from the stream
    auto quality = parsed[INDEX_QUALITY];
    auto preset = preset_from_quality(quality);

    RCLCPP_INFO(camera_manager_node->get_logger(), "Fisheye quality %d", quality);

    // Settings are used specifically for height and width resizing
    auto sett = settings();
    sett.use_preset(preset);
    auto square = sett.width;
    sett.height = sett.width; // Duplicate for opening ffmpeg
    // NO HEIGHT - square 360 cam.
    // FIXME implement fps filtering (genuine reason to run at 2fps)
    auto fps = sett.fps;
    // No local cam. Anyone who needs it should grab the rtsp stream on their own.
    const int camera_id = FISHEYE_ID; // Fisheye ID
    FILE* pipe = nullptr;

    auto cap = cv::VideoCapture(FISHEYE_URL);

    while (true) {
        // Check whether the camera is supposed to go away every iteration
        if (cam_command_map.find(camera_id) != cam_command_map.end()) {
            auto cmd = cam_command_map[camera_id];

            if (cmd[AUX_INDEX_BASE] == COMMAND_MAP_END) {
                cap.release();
                cam_command_map.erase(camera_id);
                threads_end.push_back(tmap_index);
                return;
            }
        }

        if (pipe == nullptr) {
            pipe = ffmpeg_stream_camera(sett, camera_id);
            if (pipe == nullptr) {
                RCLCPP_ERROR(camera_manager_node->get_logger(), "360 failed TRY AGAIN");
                threads_end.push_back(tmap_index);
                return;
            }
        }

        auto frame = cv::Mat();
        cap >> frame;
        if (frame.empty()) {
            // Can't spam the terminal
            cap.release();
            try {
                cap.open(FISHEYE_URL);
            } catch (const std::exception& e) {}
            continue; // Just go next regardless
        }

        // Resize the frame
        if (frame.cols != square || frame.rows != square) {
            cv::resize(frame, frame, cv::Size(square, square));
        }

        // TODO frame filtering (this one requires timing)
        if (pipe != nullptr) {
            fwrite(frame.data, 1, frame.total() * frame.elemSize(), pipe);
        }
    }
}