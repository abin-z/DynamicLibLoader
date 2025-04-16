## 跨平台动态库显式加载案例

本项目主要实现了**跨平台的显式加载动态库**的功能, 功能比较简单, 主要是记录动态库的显式加载流程.

- 封装了跨平台的动态库显式加载模块: [`application/dynamic_library.hpp`](./application/dynamic_library.hpp)
- 跨平台的动态库导出头文件: [`dynamic/include/dynamic/dll_export.h`](./dynamic/include/dynamic/dll_export.h)
- [`dynamic`](dynamic/)目录是一个独立动态库模块(比较简单).
- [`application`](application/)目录是独立程序, 其中会在代码中显式加载`dynamic`动态库.

项目文件目录:

```sh
.
├── application
│   ├── CMakeLists.txt
│   ├── dynamic_library.hpp
│   └── main.cpp
├── docs
│   └── 动态库的加载方式介绍.md
├── dynamic
│   ├── CMakeLists.txt
│   ├── include
│   │   └── dynamic
│   │       ├── common.hpp
│   │       ├── dll_export.h
│   │       └── dynamic.h
│   └── src
│       └── dynamic.cpp
└── README.md
```

### 点击查看: [动态库加载方式介绍](./docs/动态库的加载方式介绍.md)

### 动态库的加载方式主要有以下两种

1. **显式加载 (Explicit Loading)**

   显式加载动态库是通过在运行时使用特定的函数来加载动态库，通常需要提供库的路径和库的符号名称。这种方法需要程序员在代码中明确调用库加载和符号解析函数。

   - **Windows**：使用 `LoadLibrary` 加载动态库，使用 `GetProcAddress` 获取库中函数的地址。
   - **Linux/Unix**：使用 `dlopen` 加载动态库，使用 `dlsym` 获取库中函数的地址。
   - 这种方式通常用于插件系统、动态加载模块等场景。

2. **隐式加载 (Implicit Loading)**

   隐式加载是指在程序启动时，操作系统会根据可执行文件的要求自动加载所依赖的动态库。程序员不需要显式地调用加载函数，只需要在程序的构建过程中指定库的依赖。

   - **Windows**：通过 `dllimport` 声明来隐式加载动态库，操作系统会在启动时根据需要加载动态库。
   - **Linux/Unix**：通过 `ld` (链接器) 指定库依赖，操作系统会在运行时自动加载。

### 显式加载和隐式加载使用场景

- **隐式加载**适用于：依赖固定且少变、无需动态选择库的场景，简化了开发和维护工作。
- **显式加载**适用于：需要动态加载库、插件或模块化设计、按需选择库、热更新等场景，提供更大的灵活性和控制力。