cmake_minimum_required(VERSION 3.14)

project(radix_tree CXX)

add_library(radix_tree INTERFACE)

target_include_directories(radix_tree INTERFACE include)
target_compile_features(radix_tree INTERFACE cxx_std_11)

# disable testing when it is used as submodule
if(NOT CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(BUILD_TESTING OFF)
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
