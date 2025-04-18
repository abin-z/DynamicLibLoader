## Header-only的跨平台动态库显式加载器

本项目主要实现了**跨平台的显式加载动态库**的功能, 功能比较简单, 主要是记录动态库的显式加载流程.

- 封装了跨平台的动态库显式加载模块: [`application/dynamic_library.hpp`](./application/dynamic_library.hpp)
- 跨平台的动态库导出头文件: [`dynamic/include/dynamic/dll_export.h`](./dynamic/include/dynamic/dll_export.h)
- 加载器具体的使用案例请查看:  [`application/main.cpp`](./application/main.cpp)
- [`dynamic`](dynamic/)目录是一个独立动态库模块(目标是生成动态库`.so`或者`.dll`).
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

### 使用方法和案例

Header-only只需要将[`dynamic_library.hpp`](./application/dynamic_library.hpp)文件拷贝至你的项目文件夹`#include "dynamic_library.hpp"`即可使用.

<details>
  <summary>点击查看使用案例</summary>

```cpp
#include <iostream>

#include "dynamic_library.hpp"

/*
 * 为了在没有头文件的情况下调用 libdynamic.so 中的内容，你需要使用 动态链接库的运行时加载机制，
 * 即通过 dlopen、dlsym 等函数（在 POSIX 系统上）或等效的方法（在 Windows 上，如 LoadLibrary 和 GetProcAddress）。
 */

/// =========== 定义动态库中函数指针类型 start ===========
using sayHello_func  = void (*)();                // 函数指针类型
using intAdd_func    = int (&)(int, int);         // 引用函数指针类型
using floatAdd_func  = float (&&)(float, float);  // 右值引用函数指针类型
using doubleAdd_func = double(double, double);    // 函数类型
// 动态库中的struct
struct point_t
{
  double x;
  double y;
  double z;
};

using getPoint_func = point_t (*)();
using printPoint_func = void (*)(point_t);
/// =========== 定义动态库中函数指针类型 end ===========

void func();
void testNotExistSymbol(const dll::dynamic_library &lib);
int main()
{
  std::cout << "====================================================" << std::endl;
  func();
  std::cout << "====================================================" << std::endl;
  return 0;
}

/// @brief 使用封装的动态库加载流程
void func()
{
  try
  {
    const std::string libPath =
#if defined(_WIN32) || defined(_WIN64)
      "dynamic.dll";
#else
      "./bin/libdynamic.so";
#endif

    // 加载动态库
    using dll::dynamic_library;
    dynamic_library lib0(libPath);
    dynamic_library lib1(libPath);
    // dynamic_library lib = lib0; 错误: 禁止拷贝构造
    // lib0 = lib1;               错误: 禁止拷贝赋值

    lib0 = std::move(lib1);               // 支持移动赋值
    dynamic_library lib(std::move(lib0));  // 支持移动构造

    if (lib)
    {
      std::cout << "lib is vaild." << std::endl;
    }

    // 加载函数符号
    auto sayHello   = lib.get<sayHello_func>("sayHello");
    auto intAdd     = lib.get<intAdd_func>("intAdd");
    auto floatAdd   = lib.get<floatAdd_func>("floatAdd");
    auto doubleAdd  = lib.get<doubleAdd_func>("doubleAdd");
    auto getPoint   = lib.get<getPoint_func>("getPoint");
    auto printPoint = lib.get<printPoint_func>("printPoint");

    // 直接调用函数符号
    int ret = lib.invoke<int(int, int)>("intAdd", 1, 2);
    double ret2 = lib.invoke<double(double, double)>("doubleAdd", 1.8, 2.5);
    ret = lib.invoke<int(int, int)>("intAdd", 2, 3);
    ret = lib.invoke<int(int, int)>("intAdd", 3, 4);
    ret = lib.invoke<int(*)(int, int)>("intAdd", 4, 5);
    ret = lib.invoke<int(*)(int, int)>("intAdd", 5, 6);
    ret = lib.invoke<int(&)(int, int)>("intAdd", 6, 7);
    ret = lib.invoke<int(&)(int, int)>("intAdd", 7, 8);
    ret = lib.invoke<int(&&)(int, int)>("intAdd", 8, 9);
    std::cout << "invoke: intAdd(8, 9) = " << ret << std::endl;
    std::cout << "invoke: doubleAdd(1.8, 2.5) = " << ret2 << std::endl;

    // 调用函数
    sayHello();

    int a = 5, b = 3;
    std::cout << "intAdd(" << a << ", " << b << ") = " << intAdd(a, b) << std::endl;

    float fa = 1.5f, fb = 2.3f;
    std::cout << "floatAdd(" << fa << ", " << fb << ") = " << floatAdd(fa, fb) << std::endl;

    double da = 3.14159, db = 2.71828;
    std::cout << "doubleAdd(" << da << ", " << db << ") = " << doubleAdd(da, db) << std::endl;

    point_t p = getPoint();
    std::cout << "getPoint() = {x: " << p.x << ", y: " << p.y << ", z: " << p.z << "}" << std::endl;

    std::cout << "printPoint() output: ";
    printPoint(p);
    std::cout << std::endl;

    testNotExistSymbol(lib);
  }
  catch (const std::exception &ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    return;
  }
}

/// @brief 测试符号信息不存在的情况
void testNotExistSymbol(const dll::dynamic_library &lib)
{
  std::cout << "---------testNotExistSymbol----------" << std::endl;
  // 测试不存在的函数符号加载
  auto unknownFunc = lib.try_get<printPoint_func>("notExistFunc");  // 加载失败不抛异常,返回nullptr
  if (unknownFunc == nullptr)
  {
    std::cout << "lib.try_get<printPoint_func>(\"notExistFunc\"); load failed, return nullptr." << std::endl;
  }
  try
  {
    auto unknownFunc2 = lib.get<printPoint_func>("notExistFunc");  // 加载失败抛出异常
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }

  try
  {
    double ret = lib.invoke<double(double, double)>("doubleAdd", 1.5, 3.0);  // 正常调用: symbol是对的, 函数签名也正常
    std::cout << "lib.invoke ret = " << ret << std::endl;
    double ret2 = lib.invoke<double(double, double, double)>("doubleAdd", 1.5, 3.0, 1.0);  // 未定义行为: symbol是对的,但是函数签名不一致
    std::cout << "lib.invoke ret2 = " << ret2 << std::endl;
    double ret3 = lib.invoke<double()>("doubleAdd");  // 未定义行为: symbol是对的,但是函数签名不一致
    std::cout << "lib.invoke ret3 = " << ret3 << std::endl;
  }
  catch (const std::exception &e)
  {
    std::cerr << "invoke error: " << e.what() << '\n';
  }

  std::cout << "---------testNotExistSymbol----------" << std::endl;
}
```

</details>

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



