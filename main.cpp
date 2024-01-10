// stdlib
#include <cstdlib>
#include <memory>
// boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// Camera
#include "camera.hpp"
#include "camera_manager.hpp"

int main(void)
{
    // Turn off annoying libcamera logging
    setenv("LIBCAMERA_LOG_LEVELS", "3", 1);
    // Parse config file
    boost::property_tree::ptree config_ini;
    boost::property_tree::ini_parser::read_ini("config.ini", config_ini);
    std::string camera_name = config_ini.get<std::string>("Camera.Name");
    std::unique_ptr<Camera> camera = CameraManager::get().get_camera(camera_name);
    camera->request_frames();
    return 0;
}

