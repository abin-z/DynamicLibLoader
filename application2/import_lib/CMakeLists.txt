# ============================================================
#  该 CMakeLists.txt 用于引入一个“已存在”的动态库 dynamic (该方法也可以导入静态库)
#  注意:不会重新编译该库, 只是告诉 CMake 它在哪里、怎么用
# ============================================================

# 设置目标名称（变量形式, 方便后续引用）
set(tgt_name dynamic)

# ------------------------------------------------------------
# 声明一个“已存在”的共享库 (IMPORTED)
# ------------------------------------------------------------
# 说明:
#   - SHARED : 表示这是一个动态库 (.so / .dll)
#   - IMPORTED : 表示这个库不是在当前工程中编译的, 而是已经存在的外部库(无论动态或静态)
#   - GLOBAL : 让该库在所有子目录中都能被使用（可选）
# ------------------------------------------------------------
add_library(${tgt_name} SHARED IMPORTED GLOBAL)

# ------------------------------------------------------------
# 指定该外部库的实际路径（不同平台下不同）
# ------------------------------------------------------------
# IMPORTED_LOCATION:
#   告诉 CMake 这个库的真实文件路径（即 .so、.dll、.dylib）
#   仅用于“IMPORTED”库, 表示它在系统中的实际位置。
#
# IMPORTED_IMPLIB:
#   仅 Windows 使用, 对应导入库 (.lib) 文件的路径。
#------------------------------------------------------------
# 平台说明:
# Windows:
#   - .dll : 程序运行时加载的动态库文件, 运行时需要
#   - .lib : 链接时使用的“导入库”, 其中包含符号信息, 编译时需要链接该文件, 运行时不需要
#
# Linux / macOS:
#   - 只需要指定 .so 或 .dylib 文件路径即可
# 注意：
#   这些路径仅用于“编译与链接”阶段, 
#   程序运行时仍需确保动态库能被系统找到
#   (例如放在可执行文件同目录、或在 LD_LIBRARY_PATH 中)
# ------------------------------------------------------------
if(WIN32)
    set_target_properties(${tgt_name} PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/dynamic.dll"   # 运行时动态库文件路径, 运行时需要
        IMPORTED_IMPLIB   "${CMAKE_CURRENT_LIST_DIR}/lib/dynamic.lib"   # Windows 下链接时的导入库路径, 仅编译时需要
    )
else()
    set_target_properties(${tgt_name} PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libdynamic.so" # Linux 下的动态库路径
    )
endif()

# ------------------------------------------------------------
# 设置头文件搜索路径 (INTERFACE_INCLUDE_DIRECTORIES)
# ------------------------------------------------------------
# INTERFACE_INCLUDE_DIRECTORIES:
#   - 定义此目标暴露给外部的头文件目录.
#   - 也就是说, 当其他 target 使用:
#         target_link_libraries(<target_name> PRIVATE dynamic)
#     时, CMake 会自动为 app 添加该目录到编译器的 -I 选项中
# ------------------------------------------------------------
set_target_properties(${tgt_name} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include"
)

# ============================================================
# 使用说明:
#   在其他 CMakeLists.txt 中只需写:
#       target_link_libraries(<target_name> PRIVATE dynamic)
#
#   即可自动:
#       - 链接到 libdynamic.so / dynamic.dll
#       - 自动添加 include 路径
# ============================================================
