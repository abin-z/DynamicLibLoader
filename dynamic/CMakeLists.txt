# 指定CMake最低版本要求
cmake_minimum_required(VERSION 3.20)

# 设置默认构建类型为Release，如果未指定构建类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# 设置C++标准版本为C++17，并确保严格要求此标准，不使用扩展功能
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出路径：库文件、可执行文件和归档文件的生成目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build_output/lib) # 静态库 (.a/.lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build_output/bin) # 动态库 (.so/.dll)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build_output/bin) # 可执行文件

# 声明项目名称和支持的语言，同时指定版本号
project(dynamicLib LANGUAGES C CXX VERSION 0.1.1)

# 定义目标库名称
set(tgt_name dynamic)

# 创建动态库
add_library(${tgt_name} SHARED)

# 动态查找头文件和源文件，便于管理新增文件
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h include/*.hpp) # 查找所有头文件
file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.c src/*.cpp src/*.cc src/*.cxx) # 查找所有源文件

# 将头文件和源文件添加到目标中
target_sources(${tgt_name} PUBLIC ${headers}) # 头文件作为公共接口，供其他项目可见
target_sources(${tgt_name} PRIVATE ${sources}) # 源文件仅供内部使用

# 设置位置无关代码 (PIC)，对于动态库来说是必须的
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 针对Windows平台定义符号，用于导出动态库的接口
if(WIN32)
  target_compile_definitions(${tgt_name} PRIVATE DLL_PUBLIC_EXPORTS)
endif()

# 设置头文件的包含路径，供其他项目使用此库时可见
target_include_directories(${tgt_name} PUBLIC include)
