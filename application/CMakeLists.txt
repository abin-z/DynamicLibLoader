cmake_minimum_required(VERSION 3.20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin) # 可执行文件输出路径

project(app LANGUAGES C CXX VERSION 0.1)
add_executable(application main.cpp dynamicLibrary.hpp)

