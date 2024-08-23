#include "camera.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <unistd.h>

#include <boost/gil/extension/io/jpeg.hpp>

Camera::Camera(std::shared_ptr<libcamera::Camera> camera)
    : m_camera(camera), m_queued_requests(0)
{
    //std::cout << "Acquiring camera." << std::endl;
    m_camera->acquire();
    //std::cout << "Generating camera config." << std::endl;
    //m_camera_config = m_camera->generateConfiguration({ libcamera::StreamRole::Viewfinder });
    //m_camera_config = m_camera->generateConfiguration({ libcamera::StreamRole::StillCapture });
    m_camera_config = m_camera->generateConfiguration({ libcamera::StreamRole::Raw });
    //std::cout << "Validating camera config." << std::endl;
    m_camera_config->validate();
    //std::cout << "Setting camera config." << std::endl;
    m_camera->configure(m_camera_config.get());
    m_stream_config = &m_camera_config->at(0);
    std::cout << "Pixel Format: '" << m_stream_config->pixelFormat << "'." << std::endl;
    std::cout << "       Width: " << m_stream_config->size.width << std::endl;
    std::cout << "      Height: " << m_stream_config->size.height << std::endl;
    m_allocator = std::make_unique<libcamera::FrameBufferAllocator>(m_camera);
    //std::cout << "Allocating frame buffers." << std::endl;
    allocate_frame_buffers();
    /*
     * this doesn't work
     *
    std::cout << "Camera Controls:" << std::endl;
    std::cout << m_camera->controls() << std::endl;
    std::cout << "Camera Properties:" << std::endl;
    std::cout << m_camera->properties() << std::endl;
    */
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
    //std::cout << "Starting camera." << std::endl;
    m_camera->start();
    m_camera->requestCompleted.connect(this, &Camera::request_complete_handler);
    // Lock Mutex
    m_mutex.lock();
    //std::cout << "Queueing requests." << std::endl;
    for (std::unique_ptr<libcamera::Request> &request : m_requests)
    {
        m_camera->queueRequest(request.get());
        m_queued_requests++;
    }
    if (m_queued_requests > 0)
    {
        // block until all queued requests have been completed
        m_mutex.lock();
    }
    // release mutex to allow more requests
    m_mutex.unlock();
}

void Camera::request_complete_handler(libcamera::Request *request)
{
    if (request->status() == libcamera::Request::RequestCancelled)
    {
        throw std::runtime_error("Request cancelled.");
    }
    std::cout << "Request Completed." << std::endl;
    std::cout << request << std::endl;
    const std::map<const libcamera::Stream *, libcamera::FrameBuffer *> &buffers = request->buffers();
    int ibuff = 0;
    for (std::pair<const libcamera::Stream *, libcamera::FrameBuffer *> bufferPair : buffers)
    {
        std::cout << "Buffer #" << ++ibuff << std::endl;
        libcamera::FrameBuffer *buffer = bufferPair.second;
        // Metadata
        const libcamera::FrameMetadata &metadata = buffer->metadata();
        std::cout << "    seq: " << std::setw(6) << std::setfill('0') 
            << metadata.sequence << " bytesused: ";
        unsigned int nplane = 0;
        for (const libcamera::FrameMetadata::Plane &plane : metadata.planes())
        {
            std::cout << plane.bytesused;
            if (++nplane < metadata.planes().size())
            {
                std::cout << "/";
            }
        }
        std::cout << std::endl;
        // Process StreamRole::StillCapture
        /*
        if (buffer->planes().size() == 3)
        {
            assert(buffer->planes()[0].fd == buffer->planes()[1].fd);
            assert(buffer->planes()[0].fd == buffer->planes()[2].fd);
            int fd = buffer->planes()[0].fd.get();
            int width = m_stream_config->size.width;
            int height = m_stream_config->size.height;
            std::vector<uint8_t> yData(buffer->planes()[0].length);
            std::vector<uint8_t> uData(buffer->planes()[1].length);
            std::vector<uint8_t> vData(buffer->planes()[2].length);
            ::read(fd, yData.data(), buffer->planes()[0].length);
            ::read(fd, uData.data(), buffer->planes()[1].length);
            ::read(fd, vData.data(), buffer->planes()[2].length);
            boost::gil::gray8_planar_t yuvView(width, height,
                    boost::gil::planar_view_t(width, height, 
                        reinterpret_cast<const gray8_pixel_t*>(yData)),
                    boost::gil::planar_view_t(width / 2, height / 2, 
                        reinterpret_cast<const gray8_pixel_t*>(uData)),
                    boost::gil::planar_view_t(width / 2, height / 2, 
                        reinterpret_cast<const gray8_pixel_t*>(vData)));
            boost::gil::rgb8_image_t rgbImage(width, height);
            boost::gil::color_converted_view<rgb8_pixel_t>(yuvView, 
                    boost::gil::view(rgbImage));
            boost::gil::write_view("test.jpg", boost::gil::view(rgbView), 
                    boost::gil::jpeg_tag());
        }
        */
    }
    m_queued_requests--;
    std::cout << "m_queued_requests = " << m_queued_requests << std::endl;
    if (m_queued_requests <= 0)
    {
        m_mutex.unlock();
    }
}

void Camera::allocate_frame_buffers()
{
    int ret = m_allocator->allocate(m_stream_config->stream());
    if (ret < 0)
    {
        throw std::runtime_error("Failed to allocate frame buffers.");
    }
    size_t allocated = m_allocator->buffers(m_stream_config->stream()).size();
    //std::cout << "Allocated " << allocated << " buffers for stream." << std::endl;
}

void Camera::generate_requests()
{
    libcamera::Stream *stream = m_stream_config->stream();
    const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = 
        m_allocator->buffers(stream);
    for (auto &buffer : buffers)
    {
        //std::cout << "Creating request." << std::endl;
        std::unique_ptr<libcamera::Request> request = m_camera->createRequest();
        if (!request)
        {
            throw std::runtime_error("Failed to create camera request.");
        }
        int ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            throw std::runtime_error("Failed to set buffer for the request.");
        }
        m_requests.push_back(std::move(request));
    }
}

void Camera::save_jpeg(const uint8_t *data, int width, int height, const std::string &filename)
{
    boost::gil::rgb8c_view_t view = boost::gil::interleaved_view(
            width, height, reinterpret_cast<const boost::gil::rgb8_pixel_t*>(data), width);
    boost::gil::write_view(filename, view, boost::gil::jpeg_tag());
}
