cmake_minimum_required( VERSION 3.0 )

project( image_streaming )

if(NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE "release" )
endif()

set( ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
message( "ROOT_DIR: ${ROOT_DIR}" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ROOT_DIR}/bin.${CMAKE_BUILD_TYPE} )
set( PROJECTS_DIR ${ROOT_DIR}/projects )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${ROOT_DIR}/libs )
 
link_directories( ${ROOT_DIR}/libs )

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory( projects )
add_subdirectory( thirdparty )
