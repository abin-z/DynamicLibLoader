set(tgt_name application)
add_executable(${tgt_name} main.cpp)

# 链接到 dynamic_library 头文件库
target_link_libraries(${tgt_name} PRIVATE dynamic_library)