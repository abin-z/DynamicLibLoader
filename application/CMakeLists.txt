cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin) # 可执行文件输出路径

project(app LANGUAGES C CXX VERSION 0.1)
add_executable(application main.cpp)

# 判断平台来决定是否需要额外链接库
if(WIN32)
    message(STATUS "Compiling on Windows")
    # Windows 默认不需要额外链接 dl 库
elseif(APPLE)
    message(STATUS "Compiling on macOS")
    # macOS 上也通常不需要显式链接 dl（dlopen 是系统自带的）
elseif(UNIX)
    message(STATUS "Compiling on Linux")
    # Linux 需要手动链接 libdl
    target_link_libraries(application PRIVATE dl)
endif()

