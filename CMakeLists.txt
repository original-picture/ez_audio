cmake_minimum_required(VERSION 3.22)

project(ez_audio_project VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

add_library(ez_audio ez_dummy_source_file.cpp) # kind of a hack... but there will be source files eventually, and this is the easiest way to do things for now...

if(NOT TARGET ez_utils)
    add_subdirectory(lib/ez_utils)
endif()

target_include_directories(ez_audio PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(ez_audio PUBLIC ez_utils)