cmake_minimum_required(VERSION 3.0.0)
project(meeting VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_BUILD_TYPE Release)
add_definitions("-Wall -g")
# 本项目的头文件包含路径
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/include
)

# 源文件, 建立变量SRC_FILE为目录下.c文件列表
file(
    GLOB HEADER_FILE
    ${PROJECT_SOURCE_DIR}/include/*.h
) 

file(
    GLOB SRC_FILE 
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

message(STATUS ${SRC_FILE})
# 设置可执行文件输出路径
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 编译可执行文件
add_executable (${PROJECT_NAME} ${SRC_FILE} ${HEADER_FILE}) 

# 链接库
target_link_libraries (${PROJECT_NAME} pthread rt)