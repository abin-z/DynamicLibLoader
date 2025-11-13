set(tgt_name application2)
add_executable(${tgt_name} main.cpp)

# 链接到 dynamic 库（隐式加载）
target_link_libraries(${tgt_name} PRIVATE dynamic)


# -----------------------------
# 跨平台运行时库路径设置（rpath）
# -----------------------------
if(UNIX)
    # 在 Linux/Unix 系统下，设置可执行文件运行时查找共享库 (.so) 的路径。
    #
    # BUILD_RPATH: 构建/运行时使用，确保在 cmake build 过程中直接运行可执行文件时能够找到依赖的 .so
    # INSTALL_RPATH: 安装后使用，告诉系统 loader 去哪里寻找共享库，一般和 BUILD_RPATH 相同
    #
    # "$ORIGIN" 是一个特殊变量，表示“可执行文件所在目录”。
    # 也就是说 loader 会在 exe 所在目录查找 .so 文件，而不是系统默认路径。
    #
    # 这种方式适合将 exe 和 .so 放在同一目录或者相对目录下，避免每次都手动设置 LD_LIBRARY_PATH
    set_target_properties(${tgt_name} PROPERTIES
        BUILD_RPATH "$ORIGIN"     # 构建时在 exe 所在目录查找 .so
        INSTALL_RPATH "$ORIGIN"   # 安装后在 exe 所在目录查找 .so
    )
endif()  # Windows 下无需额外设置，dll 只需放在 exe 同目录即可


# -----------------------------
# 文件拷贝：将动态库拷贝到可执行文件同目录
# -----------------------------

# 根据平台选择匹配的动态库文件(根据实际情况添加)
if(WIN32)
    # Windows 下匹配所有 DLL 文件
    file(GLOB DYN_LIBS "${CMAKE_CURRENT_LIST_DIR}/../import_lib/lib/*.dll")
else()
    # Linux / macOS 下匹配所有 .so 文件
    file(GLOB DYN_LIBS "${CMAKE_CURRENT_LIST_DIR}/../import_lib/lib/*.so")
endif()

# 遍历所有匹配到的动态库文件
foreach(lib ${DYN_LIBS})
    # 添加自定义构建命令：在可执行文件构建完成后执行
    add_custom_command(
        TARGET ${tgt_name}          # 目标可执行文件
        POST_BUILD                  # 在构建完成后执行
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  # 调用 CMake 内置命令拷贝文件，如果目标不存在或不同则拷贝
            "${lib}"                # 源文件：当前循环的动态库
            $<TARGET_FILE_DIR:${tgt_name}>  # 目标路径：可执行文件所在目录
    )
endforeach()

