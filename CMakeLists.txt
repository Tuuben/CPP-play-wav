cmake_minimum_required(VERSION 3.13)

project(play-wav-file)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

find_library(AudioToolbox_LIBRARY AudioToolbox REQUIRED)

# Check if the framework was found
if (AudioToolbox_LIBRARY)
    message(STATUS "Found AudioToolbox: ${AudioToolbox_LIBRARY}")
else()
    message(FATAL_ERROR "AudioToolbox framework not found")
endif()

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(exe ${SOURCE_FILES})

target_link_libraries(exe PRIVATE ${AudioToolbox_LIBRARY})