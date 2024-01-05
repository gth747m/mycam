// stdlib
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
// libcamera
#include <libcamera/libcamera.h>
// boost
//#include <boost/process/detail/traits/wchar_t.hpp>
//#include <boost/process/env.hpp>

int main(void)
{
    setenv("LIBCAMERA_LOG_LEVELS", "3", 1);
    static std::shared_ptr<libcamera::Camera> camera;
    std::unique_ptr<libcamera::CameraManager> cm = std::make_unique<libcamera::CameraManager>();
    cm->start();
    auto cameras = cm->cameras();
    if (cameras.empty())
    {
        std::cerr << "Failed to find any cameras." << std::endl;
        cm->stop();
        return 1;
    }
    else
    {
        std::cout << "Found Cameras:" << std::endl;
        for (auto const &camera : cameras)
        {
            std::cout << "  " << camera->id() << std::endl;
        }
    }
    return 0;
}
