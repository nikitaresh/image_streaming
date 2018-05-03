
cmake_minimum_required( VERSION 3.0 )
project( pi_camera_client )

set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_AUTOMOC ON )

find_package( Qt5Core REQUIRED )
find_package( Qt5Network REQUIRED )
find_package( OpenCV REQUIRED )

include_directories( sources )
include_directories( ${ROOT_DIR}/include )
include_directories( ${Qt5Core_INCLUDE_DIRS} )
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${ROOT_DIR}/thirdparty/raspicam-0.1.6/src )

set( HEADER sources/pi_camera.h
            sources/camera_client.h
            sources/camera_client.h
            )

set( SOURCES sources/main.cpp
             sources/pi_camera.cpp
             sources/camera_client.cpp
             )

set( pi_cam_client_targets Qt5::Core Qt5::Network ${OpenCV_LIBS} )
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wl,--no-as-needed")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_C_FLAGS} ")
    SET(CMAKE_CXX_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE}  -lpthread")
    SET(CMAKE_CXX_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG}  -lpthread")

    set( pi_cam_client_targets ${pi_cam_client_targets} raspicam -lpthread )
endif(UNIX)

add_executable( pi_camera_client ${HEADER} ${SOURCES} )
target_link_libraries( pi_camera_client ${pi_cam_client_targets} )