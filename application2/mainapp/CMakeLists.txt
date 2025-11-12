add_executable(application2 main.cpp)

# linux 下使用 dl 库, Windows 默认不需要额外链接 dl 库
target_link_libraries(application2 PRIVATE ${CMAKE_DL_LIBS})

