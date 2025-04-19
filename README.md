# DynamicLibLoader 跨平台动态库加载器

**DynamicLibLoader** 是一个跨平台的动态库加载器，提供了一个统一的 API 来显式加载动态库并调用其中的符号（函数）。支持 Windows 和 POSIX 系统（如 Linux、macOS），自动处理平台差异。

### 核心特性概览:

- ✅ **跨平台支持**: 兼容 Windows 和 POSIX（Linux/macOS）.

- ✅ **RAII 资源管理**: 动态库在构造时加载，在析构时卸载，避免资源泄露.

- ✅ **异常安全**: 加载库或符号失败时抛出详细的 `std::runtime_error`，附带平台错误信息.

- **✅ 符号缓存**：`invoke()`支持符号缓存，提高符号加载效率.

- ✅ **线程安全缓存**: `invoke()`成功调用后会缓存已加载的符号指针，提高调用效率.

- ✅ **缓存与非缓存调用接口**: 提供 `invoke()`（自动缓存）和 `invoke_uncached()`（不缓存）两种调用方式.

- **✅ 无依赖**：仅依赖标准库，不依赖任何外部库.

- ✅ **支持多种函数类型:**

  | 类型名称             | 定义形式      | 说明                 |
  | -------------------- | ------------- | -------------------- |
  | 函数指针类型         | `T (*)(...)`  | 最常用的函数指针形式 |
  | 引用函数指针类型     | `T (&)(...)`  | 函数的左值引用类型   |
  | 右值引用函数指针类型 | `T (&&)(...)` | 函数的右值引用类型   |
  | 函数类型             | `T(...)`      | 直接使用函数类型     |

### 使用方法

Header-only只需要将[`dynamic_library.hpp`](./application/dynamic_library.hpp)文件拷贝至你的项目文件夹`#include "dynamic_library.hpp"`即可使用.

### 基础案例

1. **加载动态库并获取符号**

```cpp
#include <iostream>
#include "dynamic_library.hpp"
int main()
{
  try
  {
    dll::dynamic_library lib("path/to/your/library.so");  // 加载动态库
    auto func_add = lib.get<int(int, int)>("add");        // 获取符号(函数),此符号必须在库中存在
    func_add(1, 1);                                       // 调用符号(函数)
  }
  catch (const std::runtime_error &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
```

2. **使用 `try_get()` 尝试获取符号**

```cpp
int main()
{
  dll::dynamic_library lib("path/to/your/library.so");
  // try_get尝试获取符号，如果符号不存在则返回 nullptr
  auto func_add = lib.try_get<int(int, int)>("add");
  if (func_add)
  {
    func_add(1, 1);  // 如果符号加载成功，调用符号
  }
  else
  {
    std::cerr << "Failed to load function!" << std::endl;
  }
}
```

3. **使用 `invoke()` 简化调用过程**

```cpp
int main()
{
  try
  {
    dll::dynamic_library lib("path/to/your/library.so");
    // 直接调用名为 "add" 的函数，该函数签名为 int(int, int)
    int result = lib.invoke<int(int, int)>("add", 10, 20);
    std::cout << "Result: " << result << std::endl;
  }
  catch (const std::runtime_error &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
```

### 警告: 可能的未定义行为

在获取函数符号时，**一定要确保你传入的函数类型和库中的函数签名完全一致.**

如果提供的**函数签名**与动态库中**实际符号的签名**不一致，将导致**未定义行为**.

假设正确的 `doubleAdd` 函数签名应该是: 是一个接受两个 `double` 参数并返回一个 `double` 的函数指针类型:

```cpp
double(double, double)   // doubleAdd 的函数类型(签名)
```

```cpp
try
{
  // 正常调用: symbol 是对的, 函数签名也正常
  double ret = lib.invoke<double(double, double)>("doubleAdd", 1.5, 3.0);

  // 未定义行为: doubleAdd 只接受两个 double 参数，但此处传递了三个 double 参数，这会导致未定义行为
  double ret2 = lib.invoke<double(double, double, double)>("doubleAdd", 1.5, 3.0, 1.0);
    
  // 未定义行为: 因为 doubleAdd 符号要求两个 double 类型的参数，而调用时没有提供任何参数
  double ret3 = lib.invoke<double()>("doubleAdd");
}
catch (const std::exception &e)
{
  std::cerr << "invoke error: " << e.what() << '\n';
}
```

### 仓库目录结构介绍

- [`dynamic`](dynamic/)目录是一个独立动态库模块(目标是生成动态库`.so`或者`.dll`).
- [`application`](application/)目录是独立程序, 其中会在代码中显式加载`dynamic`动态库.

- 封装了跨平台的动态库显式加载模块: [`application/dynamic_library.hpp`](./application/dynamic_library.hpp)
- 跨平台的动态库导出头文件: [`dynamic/include/dynamic/dll_export.h`](./dynamic/include/dynamic/dll_export.h)
- 具体的使用案例请查看:  [`application/main.cpp`](./application/main.cpp)

项目文件目录:

```sh
.
├── application					# 2.使用本库加载dynamic生成的动态库
│   ├── CMakeLists.txt
│   ├── dynamic_library.hpp
│   └── main.cpp
├── docs
│   └── 动态库的加载方式介绍.md
├── dynamic						# 1.生成dynamic动态库供application调用
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

<details>
  <summary>点击查看完整使用案例</summary>


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
    double ret3 = lib.invoke_uncached<double(double, double)>("doubleAdd", 1.8, 2.5);
    std::cout << "invoke: intAdd(8, 9) = " << ret << std::endl;
    std::cout << "invoke: doubleAdd(1.8, 2.5) = " << ret2 << std::endl;
    std::cout << "invoke_uncached: doubleAdd(1.8, 2.5) = " << ret3 << std::endl;

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



