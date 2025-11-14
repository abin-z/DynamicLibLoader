## 概述

**application 为独立项目, 主要演示使用dynamic_library显示加载动态库.so或者.dll**

常见场景: 三方库只提供了 `.so/.dll` 动态库文件, 没有提供头文件. 但是我们知道其中的符号信息. 

目录结构:

```sh
.
├── CMakeLists.txt
├── README.md
├── dynamic_library                   # 动态库显式加载器
│   ├── CMakeLists.txt
│   └── include
│       └── dynamic_library
│           └── dynamic_library.hpp   # 核心头文件
└── mainapp                           # mainapp主要演示如何使用dynamic_library库
    ├── CMakeLists.txt
    └── main.cpp
```

#### 核心库: dynamic_library

**Header-Only** 头文件: [dynamic_library.hpp](dynamic_library/include/dynamic_library/dynamic_library.hpp) (仅一个头文件)

若使用cmake管理, 可以完整拷贝: [dynamic_library](dynamic_library/) 文件夹(已包含CMakeLists.txt)到你的项目

#### 使用案例: mainapp

[mainapp/main.cpp](mainapp/main.cpp) 演示了如何使用dynamic_library库显式加载动态库中的符号信息.

