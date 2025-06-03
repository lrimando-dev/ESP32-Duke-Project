# The following lines of boilerplate have to be in your project's
# CMakeLists.txt in order to use ESP-IDF:

cmake_minimum_required(VERSION 3.16)

# Set the project name
set(PROJECT_NAME "ESP32-Duke-Debugger")

# Include ESP-IDF's project configuration
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Define the project
project(${PROJECT_NAME})

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
