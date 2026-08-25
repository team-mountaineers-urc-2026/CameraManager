/**
 * @file realsense_interface.cpp
 * @author William Streck
 * @brief Realsense interface functions for the Camera Manager.
 * @version 0.1
 * @date 2025-02-22
 * 
 */

#include "realsense_interface.hpp"
#include "CameraManagerNode.hpp"

static queue<sensor_msgs::msg::Image> realsense_frame_queue;

void CameraManager::realsense_img_callback(const sensor_msgs::msg::Image::SharedPtr msg) const {
    // If and only if we are using the wrist camera, push the frame to the queue
    if (cameras.find(-1) != cameras.end()) {
        if ((*msg).data.data() != nullptr) {
            realsense_frame_queue.push(*msg);
        }
    } else {
        // ensure the queue is cleared if we are not watching the camera
        while (!realsense_frame_queue.empty()) {
            realsense_frame_queue.pop();
        }
    }
}

void realsense_cam_thread(map<string, int> parsed, int tmap_index) {
    // verify the camera ID
    if (parsed[INDEX_ID] != WRIST_ID) {
        threads_end.push_back(tmap_index);
        return;
    }

    // Get the important stuff - we WON'T be using a camera object - just receiving images
    auto quality = parsed[INDEX_QUALITY];
    auto preset = preset_from_quality(quality);

    RCLCPP_INFO(camera_manager_node->get_logger(), "Realsense quality %d", quality);

    // Settings are used specifically for height and width resizing
    auto sett = settings();
    sett.use_preset(preset);
    auto width = sett.width;
    auto height = sett.height;
    auto stream_fps = sett.fps;
    auto cam_local = false;
    auto cam_streaming = false;
    const int camera_id = WRIST_ID; // Wrist ID
    FILE* pipe = nullptr;
    int fps_filter_count = 0;
    int fps_filter = 0;
    if (stream_fps != 0) {
        fps_filter = (REALSENSE_NATIVE_FPS/stream_fps) - 1;
    }

    while (true) {
        // Grab any available frames
        while (realsense_frame_queue.size() > 0) {
            // Check if we should be running local comms
            if (!cam_local && local_cams.find(camera_id) != local_cams.end()) {
                cam_local = true;
            } else if (cam_local && local_cams.find(camera_id) == local_cams.end()) {
                cam_local = false;
            }

            // Check if we should be streaming or should close a stream
            if (!cam_streaming && streaming_cams.find(camera_id) != streaming_cams.end()) {
                pipe = ffmpeg_stream_camera(sett, camera_id);
                if (pipe == nullptr) {
                    RCLCPP_ERROR(camera_manager_node->get_logger(), "FFMPEG failure-restart Realsense");

                    // Erase from streaming cams for now
                    if (streaming_cams.find(camera_id) != streaming_cams.end()) {
                        streaming_cams.erase(camera_id);
                    }
                }

                cam_streaming = true;
            } else if (cam_streaming && streaming_cams.find(camera_id) == streaming_cams.end()) {
                cam_streaming = false;
                
                // Stop the ffmpeg process
                if (pipe != nullptr) {
                    kill(fileno(pipe), SIGKILL);
                    pclose(pipe);
                    pipe = nullptr;
                }

                // Avoid startup delays
                fps_filter_count = 0;
            }

            // Get the frame
            RCLCPP_INFO(camera_manager_node->get_logger(), "bad alloc before msg");
            auto msg = realsense_frame_queue.front();
            if (msg.data.data() == nullptr) {
                RCLCPP_ERROR(camera_manager_node->get_logger(), "Null found BEFORE resize");
            }
            RCLCPP_INFO(camera_manager_node->get_logger(), "bad alloc after msg");
            realsense_frame_queue.pop();

            // Check if we need to change the size
            if (msg.width > width && msg.height > height) {
                // Hold the old message and rescope a new one to hold new data
                auto old_msg = msg;
                msg = sensor_msgs::msg::Image();
                msg.height = height;
                msg.width = width;
                msg.encoding = old_msg.encoding; // pixel encoding
                msg.step = old_msg.step;
                
                // Resize the image
                if (old_msg.data.data() == nullptr) {
                    RCLCPP_ERROR(camera_manager_node->get_logger(), "Null found at resize");
                }
                cv::Mat frame(old_msg.height, old_msg.width, CV_8UC3, (uchar*)old_msg.data.data());
                cv::resize(frame, frame, cv::Size(width, height));
                msg.data = vector<uint8_t>(frame.data, frame.data + frame.total() * frame.elemSize());
                msg.header.stamp = old_msg.header.stamp;
                // We can just let old_msg go out of scope - the original msg will get deallocated when looping
            } else {
                if (width > REALSENSE_DEFAULT_WIDTH) {
                    width = REALSENSE_DEFAULT_WIDTH;
                }
                if (height > REALSENSE_DEFAULT_HEIGHT) {
                    height = REALSENSE_DEFAULT_HEIGHT;
                }
            }

            if (cam_local) {
                // Steal the ROS2 message - NOT SUPPORTING RESIZING OR METADATA
                msg.header.frame_id = to_string(WRIST_ID);
                camera_manager_node->publish_image(msg);
            }

            if (cam_streaming && (pipe != nullptr)) {
                if (stream_fps > 0 && fps_filter_count >= fps_filter) {
                    if (msg.data.data() == nullptr) {
                        RCLCPP_ERROR(camera_manager_node->get_logger(), "Null found at fwrite");
                    }
                    cv::Mat frame(msg.height, msg.width, CV_8UC3, (uchar*)msg.data.data());
                    cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);
                    fwrite(frame.data, 1, frame.total() * frame.elemSize(), pipe);
                    // Just like the other cams, we don't have to do anything else now!!
                    fps_filter_count = 0;
                } else {
                    fps_filter_count++;
                }
            }
        }

        // No commands except end
        if (cam_command_map.find(camera_id) != cam_command_map.end()) {
            auto cmd = cam_command_map[camera_id];

            if (cmd[AUX_INDEX_BASE] == COMMAND_MAP_END) {
                // Stop listening to the realsense. Let the realsense package do what it wants.
                cam_command_map.erase(camera_id);
                threads_end.push_back(tmap_index); // Must occur LAST
                return;
            } else if (cmd[INDEX_MODE] == ATTRIBUTE_MODIFY) {
                auto attribute = cmd[INDEX_ATTRIBUTE];
                auto value = cmd[INDEX_AT_VALUE];

                if (attribute == ATTR_STREAM_RESOLUTION && cam_streaming) {
                    // Immediately restart the ffmpeg process before another frame can even exist
                    kill(fileno(pipe), SIGTERM); // SIGTERM - graceful shutdown, may help with rtsp running
                    pclose(pipe);

                    // Set only the new resolution and not fps
                    sett.use_preset(preset_from_quality(value));
                    if (sett.width > REALSENSE_DEFAULT_WIDTH) {
                        sett.width = REALSENSE_DEFAULT_WIDTH;
                    }
                    if (sett.height > REALSENSE_DEFAULT_HEIGHT) {
                        sett.height = REALSENSE_DEFAULT_HEIGHT;
                    }
                    width = sett.width;
                    height = sett.height;

                    // New process
                    pipe = ffmpeg_stream_camera(sett, camera_id);
                    if (pipe == nullptr) {
                        RCLCPP_ERROR(camera_manager_node->get_logger(), "RS FFMPEG failed - try again");
                    }
                } else if (attribute == ATTR_LOCAL_RESOLUTION && cam_local) {
                    // Same as above but without the ffmpeg process

                    // Set only the new resolution and not fps
                    sett.use_preset(preset_from_quality(value));
                    width = sett.width;
                    height = sett.height;
                } else if (attribute == ATTR_STREAM_FPS && cam_streaming) {
                    sett.fps = value; // Not even necessary right now
                    stream_fps = value;
                    if (stream_fps != 0) {
                        fps_filter = (REALSENSE_NATIVE_FPS/stream_fps) - 1;
                    }
                }
                
            }

            cam_command_map.erase(camera_id);
        }
    }
}
