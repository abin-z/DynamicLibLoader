# =============================
# 头文件库（Header-Only）配置
# =============================
# INTERFACE 类型库：
#   - 不会生成静态库 (.a/.lib) 或动态库 (.so/.dll)
#   - 仅提供接口给依赖它的目标
#   - 适合纯头文件库或者只需暴露 include 路径的情况

set(tgt_name dynamic_library)
add_library(${tgt_name} INTERFACE)  # 声明一个 INTERFACE 库

# -----------------------------
# 头文件路径设置
# -----------------------------
# INTERFACE include path 表示：
#   - 这个路径只对依赖此库的 target 可见
#   - $<BUILD_INTERFACE:...> 确保 build 阶段使用, 相对路径安全
target_include_directories(${tgt_name} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

# -----------------------------
# 链接系统库
# -----------------------------
# Linux 下, 如果头文件里用到了 dlopen/dlsym 等函数, 需要链接 dl 库
# Windows 下 ${CMAKE_DL_LIBS} 为空, 不影响
target_link_libraries(${tgt_name} INTERFACE ${CMAKE_DL_LIBS})

# -----------------------------
# 源文件展示(优化 IDE 体验)
# -----------------------------
# 仅用于 IDE 显示头文件, 让用户在 IDE 中能看到库的结构
# 对编译没有功能性影响
target_sources(${tgt_name} INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/include/dynamic_library/dynamic_library.hpp"
)
