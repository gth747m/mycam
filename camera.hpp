#pragma once

// stdlib
#include <atomic>
#include <iostream>
#include <memory>
#include <semaphore>
#include <vector>
// libcamera
#include <libcamera/libcamera.h>

class Camera
{
public:
    Camera(std::shared_ptr<libcamera::Camera> camera);
    virtual ~Camera();
    void request_frames();
    void request_complete_handler(libcamera::Request *request);
private:
    std::shared_ptr<libcamera::Camera> m_camera;
    std::unique_ptr<libcamera::CameraConfiguration> m_camera_config;
    libcamera::StreamConfiguration *m_stream_config;
    std::unique_ptr<libcamera::FrameBufferAllocator> m_allocator;
    std::vector<std::unique_ptr<libcamera::Request>> m_requests;
    std::atomic_int m_queued_requests;
    std::binary_semaphore m_semaphore;
    void allocate_frame_buffers();
    void generate_requests();
};
