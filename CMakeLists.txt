cmake_minimum_required(VERSION 3.25.0)
project(mycam)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-ggdb")

add_executable(${PROJECT_NAME} 
    main.cpp
    camera.hpp
    camera.cpp
    camera_manager.hpp
    camera_manager.cpp
    )

find_package(Boost REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC /usr/include/libcamera ${Boost_INCLUDE_DIRS})

target_link_directories(${PROJECT_NAME} PUBLIC /usr/lib/aarch64-linux-gnu)

target_link_libraries(${PROJECT_NAME} 
    libcamera.so
    libcamera-base.so
)
