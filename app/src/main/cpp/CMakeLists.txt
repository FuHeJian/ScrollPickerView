cmake_minimum_required(VERSION 3.4.1)

project(test VERSION 1.0)
#使用c++17
#set(CMAKE_CXX_STANDARD 17)

#设置动态库的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI})

#创建动态库文件
add_library(test SHARED test.c test.cpp android_camera.c)

target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR}/src/ffmpeg/include)

get_filename_component(aaa ${PROJECT_SOURCE_DIR}/../../../libs ABSOLUTE)

message(目录：PROJECT_SOURCE_DIR：${aaa})
find_library(log log)
find_library(android android)
find_library(camera camera2ndk)
find_library(media mediandk)
# 获取指定目录下的所有.so文件
file(GLOB LIBS ${aaa}/${CMAKE_ANDROID_ARCH_ABI}/*.so)
message(安卓库 ${android})
message(安卓库 ${camera})
message(安卓库 ${media})
# 将所有.so文件链接到你的目标
foreach(LIB ${LIBS})
    message(文件 ${LIB})
    target_link_libraries(test ${LIB})
endforeach()
target_link_libraries(test ${log})
target_link_libraries(test ${android})
target_link_libraries(test ${camera})
target_link_libraries(test ${media})


