#include "camera.hpp"

#include <iomanip>

static void request_complete_handler(libcamera::Request *request)
{
    if (request->status() == libcamera::Request::RequestCancelled)
    {
        std::cerr << "Request Cancelled." << std::endl;
        return;
    }
    const std::map<const libcamera::Stream *, libcamera::FrameBuffer *> &buffers = request->buffers();
    for (std::pair<const libcamera::Stream *, libcamera::FrameBuffer *> bufferPair : buffers)
    {
        libcamera::FrameBuffer *buffer = bufferPair.second;
        const libcamera::FrameMetadata &metadata = buffer->metadata();
        std::cout << " seq: " << std::setw(6) << std::setfill('0') 
            << metadata.sequence << " bytesused: ";
        unsigned int nplane = 0;
        for (const libcamera::FrameMetadata::Plane &plane : metadata.planes())
        {
            std::cout << plane.bytesused;
            if (++nplane < metadata.planes().size())
            {
                std::cout << "/";
            }
            std::cout << std::endl;
        }
    }
}

Camera::Camera(std::shared_ptr<libcamera::Camera> camera)
    : m_camera(camera)
{
    std::cout << "Acquiring camera." << std::endl;
    m_camera->acquire();
    std::cout << "Generating camera config." << std::endl;
    m_camera_config = m_camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });
    std::cout << "Validating camera config." << std::endl;
    m_camera_config->validate();
    std::cout << "Setting camera config." << std::endl;
    m_camera->configure(m_camera_config.get());
    m_stream_config = &m_camera_config->at(0);
    m_allocator = std::make_unique<libcamera::FrameBufferAllocator>(m_camera);
    std::cout << "Allocating frame buffers." << std::endl;
    allocate_frame_buffers();
}

Camera::~Camera()
{
    m_allocator->free(m_stream_config->stream());
    m_camera->stop();
    m_camera->release();
    m_camera.reset();
}

void Camera::request_frames()
{
    generate_requests();
    std::cout << "Starting camera." << std::endl;
    m_camera->start();
    m_camera->requestCompleted.connect(request_complete_handler);
    std::cout << "Queueing requests." << std::endl;
    for (std::unique_ptr<libcamera::Request> &request : m_requests)
    {
        m_camera->queueRequest(request.get());
        m_queued_requests++;
    }
}

void Camera::allocate_frame_buffers()
{
    int ret = m_allocator->allocate(m_stream_config->stream());
    if (ret < 0)
    {
        std::cerr << "Failed to allocate frame buffers." << std::endl;
    }
    size_t allocated = m_allocator->buffers(m_stream_config->stream()).size();
    std::cout << "Allocated " << allocated << " buffers for stream." << std::endl;
}

void Camera::generate_requests()
{
    libcamera::Stream *stream = m_stream_config->stream();
    const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = 
        m_allocator->buffers(stream);
    for (auto &buffer : buffers)
    {
        std::cout << "Creating request." << std::endl;
        std::unique_ptr<libcamera::Request> request = m_camera->createRequest();
        if (!request)
        {
            std::cerr << "Failed to create camera request." << std::endl;
        }
        int ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Failed to set buffer for the request." << std::endl;
        }
        m_requests.push_back(std::move(request));
    }
}

