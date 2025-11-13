add_executable(application2 main.cpp)

# linux 下使用 dl 库, Windows 默认不需要额外链接 dl 库
target_link_libraries(application2 PRIVATE ${CMAKE_DL_LIBS})

# 链接到 dynamic 库（隐式加载）
target_link_libraries(application2 PRIVATE dynamic)

# 跨平台运行时库路径设置
if(UNIX)
    # 设置运行时 rpath，使程序在同目录可直接找到 .so
    set_target_properties(application2 PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Windows 下无需额外设置，dll 放在 exe 同目录即可
