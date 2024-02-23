cmake_minimum_required(VERSION 3.4.1)

project(test VERSION 1.0)
#使用c++17
#set(CMAKE_CXX_STANDARD 17)

#设置动态库的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI})

#创建动态库文件
message(当前目录：PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE MYSRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE MYSRCC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(test SHARED ${MYSRC} ${MYSRCC} test.c test.cpp android_camera.c hook.cpp)
add_library(test2 SHARED ddd.cpp)

target_include_directories(test PUBLIC ffmpeg)

get_filename_component(aaa ${PROJECT_SOURCE_DIR}/../../../libs ABSOLUTE)

message(目录：PROJECT_SOURCE_DIR：${aaa})
find_library(log log)
find_library(android android)
find_library(camera camera2ndk)
find_library(media mediandk)
find_library(sles opensles)
find_library(dl dl)
# 获取指定目录下的所有.so文件
file(GLOB LIBS ${aaa}/${CMAKE_ANDROID_ARCH_ABI}/*.so)
message(安卓库 ${android})
message(安卓库 ${camera})
message(安卓库 ${media})
message(dl安卓库 ${dl})
# 将所有.so文件链接到你的目标
foreach(LIB ${LIBS})
    message(文件 ${LIB})
    target_link_libraries(test ${LIB})
endforeach()

target_link_libraries(test ${log})
target_link_libraries(test ${android})
target_link_libraries(test ${camera})
target_link_libraries(test ${media})
target_link_libraries(test ${sles})
target_link_libraries(test ${dl})