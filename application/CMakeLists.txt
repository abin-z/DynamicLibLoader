cmake_minimum_required(VERSION 3.20)

# 开启生成 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin) # 可执行文件输出路径

project(app LANGUAGES C CXX VERSION 0.1)
add_executable(application main.cpp)

# linux 下使用 dl 库, Windows 默认不需要额外链接 dl 库
target_link_libraries(application PRIVATE ${CMAKE_DL_LIBS})

