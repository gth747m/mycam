#include "camera_manager.hpp"

#include <iostream>

CameraManager::CameraManager()
{
}

CameraManager &CameraManager::get()
{
    if (m_instance == nullptr)
    {
        std::cout << "Creating CameraManager." << std::endl;
        m_instance = new CameraManager();
        m_instance->m_libcamera_camera_manager = std::make_unique<libcamera::CameraManager>();
        std::cout << "Starting CameraManager." << std::endl;
        m_instance->m_libcamera_camera_manager->start();
    }
    return *m_instance;
}

void CameraManager::destruct()
{
    std::cout << "Stopping CameraManager." << std::endl;
    m_instance->m_libcamera_camera_manager->stop();
    std::cout << "Deleting CameraManager." << std::endl;
    delete m_instance;
    m_instance = nullptr;
}

std::unique_ptr<Camera> CameraManager::get_camera(const std::string &camera_name)
{
    std::cout << "Creating Camera." << std::endl;
    return std::move(std::make_unique<Camera>(m_libcamera_camera_manager->get(camera_name)));
}

CameraManager* CameraManager::m_instance = nullptr;
