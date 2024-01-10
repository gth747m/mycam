// stdlib
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
// libcamera
#include <libcamera/libcamera.h>
// boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// Camera
#include "camera_manager.hpp"
#include "camera.hpp"

/*
std::atomic_int filled_requests = 0;
static void request_complete(Request *request);
*/

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
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    return 0;
    /*
    // Create the camera manager
    static std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();
    // Find the camera specified in the ini file
    std::cout << "Looking for camera '" << camera_name << "'." << std::endl;
    std::shared_ptr<Camera> camera = cm->get(camera_name);
    if (!camera)
    {
        std::cerr << "Failed to find camera." << std::endl;
        cm->stop();
        return 1;
    }
    // Get exclusive access to the camera
    std::cout << "Found" << std::endl;
    std::cout << "Configuring..." << std::endl;
    camera->acquire();
    // Get default comfigurations
    std::unique_ptr<CameraConfiguration> camera_config = 
        camera->generateConfiguration({ StreamRole::Viewfinder });
        //camera->generateConfiguration({ StreamRole::StillCapture });
        //camera->generateConfiguration({ StreamRole::Viewfinder, StreamRole::StillCapture });
    if (!camera_config)
    {
        std::cerr << "Failed to get camera config." << std::endl;
        cm->stop();
        return 1;
    }
    // Configure
    CameraConfiguration::Status validation_status = camera_config->validate();
    std::cout << "Camera configuration validation status = '" 
        << validation_status << "'." << std::endl;
    int configuration_status = camera->configure(camera_config.get());
    std::cout << "Camera configuration status = '" 
        << configuration_status << "'." << std::endl;
    // Allocate frame buffers
    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);
    for (StreamConfiguration &stream_config : *camera_config)
    {
        int ret = allocator->allocate(stream_config.stream());
        if (ret < 0)
        {
            std::cerr << "Failed to allocate buffers." << std::endl;
            return -ENOMEM;
        }
        size_t allocated = allocator->buffers(stream_config.stream()).size();
        std::cout << "Allocated " << allocated << " buffers for stream '" 
            << stream_config.toString() << "'." << std::endl;
    }
    // Create requests
    camera->requestCompleted.connect(request_complete);
    std::vector<std::unique_ptr<Request>> requests;
    for (StreamConfiguration &stream_config : *camera_config)
    {
        Stream *stream = stream_config.stream();
        const std::vector<std::unique_ptr<FrameBuffer>> &buffers = 
            allocator->buffers(stream);
        for (unsigned int i = 0; i < buffers.size(); i++)
        {
            std::unique_ptr<Request> request = camera->createRequest();
            if (!request)
            {
                std::cerr << "Failed to create camera request." << std::endl;
                return -ENOMEM;
            }
            const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
            int ret = request->addBuffer(stream, buffer.get());
            if (ret < 0)
            {
                std::cerr << "Failed to set buffer for request." << std::endl;
                return ret;
            }
            requests.push_back(std::move(request));
        }
    }
    camera->start();
    std::atomic_int request_count = 0;
    for (StreamConfiguration &stream_config : *camera_config)
    {
        for (std::unique_ptr<Request> &request : requests)
        {
            camera->queueRequest(request.get());
            request_count++;
        }
    }
    while (filled_requests < request_count)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    // Cleanup
    camera->stop();
    for (StreamConfiguration &stream_config : *camera_config)
    {
        Stream *stream = stream_config.stream();
        allocator->free(stream);
    }
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();
    return 0;
    */
}

/*
static void request_complete(Request *request)
{
    if (request->status() == Request::RequestCancelled)
    {
        std::cerr << "Request Cancelled." << std::endl;
        return;
    }
    std::cout << "Request Complete: '" << request->toString() << "'." << std::endl;
    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();
    for (std::pair<const Stream *, FrameBuffer *> bufferPair : buffers)
    {
        FrameBuffer *buffer = bufferPair.second;
        const FrameMetadata &metadata = buffer->metadata();
        std::cout << " seq: " << std::setw(6) << std::setfill('0') 
            << metadata.sequence << " bytesused: ";
        unsigned int nplane = 0;
        for (const FrameMetadata::Plane &plane : metadata.planes())
        {
            std::cout << plane.bytesused;
            if (++nplane < metadata.planes().size())
            {
                std::cout << "/";
            }
            std::cout << std::endl;
        }
    }
    filled_requests++;
}
*/

