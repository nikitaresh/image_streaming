# Image Streaming
Streaming images from Raspberry Pi Camera to Qt Viewer

This repository represents streaming images from Raspberry Pi to remoute application through the server. The System consist of three projects:

[Server](#server)

[Raspberry Pi application](#raspberry-pi-application)

[Viewer application](#viewer)


## Server
Server application locates in [image_streaming/projects/server/](projects/server/) directory. This console application provides connection of camera and viewer clients, transfer of control commands and transferring images from a camera to clients. 
Server project was written in C++ and depends only on Qt5 library, so you can build this application on different Operating Systems, which support Qt5 library.
Before running the server, make sure that the server port (default value 555) is opened: an exception has been added to the firewall (on Windows OS) and the server port is accessible from Viewer and Raspberry Pi networks.

## Raspberry Pi application
The pi_camera_client application locates in [image_streaming/projects/pi_camera_client/](projects/pi_camera_client/) directory. This console application provides image transfer from the Raspberry Pi Camera to the Server.
The pi_camera_client was written in C++ and depends on Qt5, OpenCV and raspicam libraries. The raspicam library version 0.1.6 already located in [image_streaming/thirdparty/raspicam-0.1.6/](thirdparty/raspicam-0.1.6/) directory.

**Build steps for Raspberry Pi with Ubuntu MATE OS**:
1. Connect the Camera to Raspberry Pi, and set the value "Enable" for the Camera in the config mode:

       sudo raspi-config

2. Install CMake, Qt5 and OpenCV libraries:

       sudo apt install cmake
       sudo apt-get install qt5-default
       sudo apt-get install libopencv-dev

3. Extract image_streaming project to home directory. Build and run pi_camera_client:

       cd ~/image_streaming/build
       cmake ..
       make
       cd ~/image_streaming/bin.release
       ./pi_camera_client

**Configuring automatic starting pi_camera_client at Raspberry Pi boot**:
1. Copy pi_camera_client executable to system directory:

       sudo cp ~/image_streaming/bin.release/pi_camera_client /usr/sbin/pi_camera_client   
2. Copy pi_camera_client service and configure it:

       sudo cp ~/image_streaming/projects/pi_camera_client/service/pi_camera_client /etc/init.d/
       sudo chmod +x /etc/init.d/pi_camera_client    
       sudo update-rc.d pi_camera_client defaults

## Viewer
The Viewer application locates in [image_streaming/projects/viewer/](projects/viewer/) directory. This GUI application provides displaying images from a Raspberry Pi Camera.
The Viewer application was written in C++ and depends on Qt5 library, includin Widgets module.
This application was tested and can be used in Windows and Android OS.
