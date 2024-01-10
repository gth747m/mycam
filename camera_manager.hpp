#pragma once

// stdlib
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <string_view>
// libcamera
#include <libcamera/libcamera.h>
// Camera
#include "camera.hpp"

class CameraManager
{
public:
    static CameraManager &get();
    static void destruct();
    std::unique_ptr<Camera> get_camera(const std::string &camera_name);
    CameraManager(const CameraManager &) = delete;
    CameraManager &operator=(const CameraManager &) = delete;
private:
    CameraManager();
    ~CameraManager() = default;
    std::unique_ptr<libcamera::CameraManager> m_libcamera_camera_manager;
    static CameraManager* m_instance;
};
