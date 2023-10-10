cmake_minimum_required(VERSION 3.4.1)

project(test VERSION 1.0 LANGUAGES C CXX)

#设置动态库的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI})

#创建动态库文件
add_library(test SHARED test.cpp)



