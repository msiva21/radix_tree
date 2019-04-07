cmake_minimum_required(VERSION 3.14)

project(radix_tree CXX)

add_library(radix_tree INTERFACE)

target_include_directories(radix_tree INTERFACE include)
target_compile_features(radix_tree INTERFACE cxx_std_11)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
