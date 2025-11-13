set(tgt_name dynamic)

# 声明一个已存在的外部共享库 (不重新编译)
add_library(${tgt_name} SHARED IMPORTED GLOBAL)

# 指定平台相关的动态库文件路径
if(WIN32)
    set_target_properties(${tgt_name} PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/dynamic.dll"
        IMPORTED_IMPLIB   "${CMAKE_CURRENT_LIST_DIR}/lib/dynamic.lib"  # Windows 下使用
    )
else()
    set_target_properties(${tgt_name} PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libdynamic.so"
    )
endif()

# 添加头文件路径
set_target_properties(${tgt_name} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include"
)
