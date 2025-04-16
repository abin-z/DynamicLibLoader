#include <iostream>

#include "dynamic_library.hpp"

/*
 * 为了在没有头文件的情况下调用 libdynamic.so 中的内容，你需要使用 动态链接库的运行时加载机制，
 * 即通过 dlopen、dlsym 等函数（在 POSIX 系统上）或等效的方法（在 Windows 上，如 LoadLibrary 和 GetProcAddress）。
 */

/// =========== 定义动态库中函数指针类型 start ===========
using sayHello_func = void (*)();
using intAdd_func = int (*)(int, int);
using floatAdd_func = float (*)(float, float);
using doubleAdd_func = double (*)(double, double);
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
void testNotExistSymbol(const dll::DynamicLibrary &lib);
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
    using dll::DynamicLibrary;
    DynamicLibrary lib0(libPath);
    DynamicLibrary lib1(libPath);
    // DynamicLibrary lib = lib0; 错误: 禁止拷贝构造
    // lib0 = lib1;               错误: 禁止拷贝赋值

    lib0 = std::move(lib1);               // 支持移动赋值
    DynamicLibrary lib(std::move(lib0));  // 支持移动构造

    // 加载函数符号
    auto sayHello   = lib.loadSymbol<sayHello_func>("sayHello");
    auto intAdd     = lib.loadSymbol<intAdd_func>("intAdd");
    auto floatAdd   = lib.loadSymbol<floatAdd_func>("floatAdd");
    auto doubleAdd  = lib.loadSymbol<doubleAdd_func>("doubleAdd");
    auto getPoint   = lib.loadSymbol<getPoint_func>("getPoint");
    auto printPoint = lib.loadSymbol<printPoint_func>("printPoint");

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
void testNotExistSymbol(const dll::DynamicLibrary &lib)
{
  std::cout << "---------testNotExistSymbol----------" << std::endl;
  // 测试不存在的函数符号加载
  auto unknownFunc = lib.tryLoadSymbol<printPoint_func>("notExistFunc");  // 加载失败不抛异常,返回nullptr
  if (unknownFunc == nullptr)
  {
    std::cout << "lib.tryLoadSymbol<printPoint_func>(\"notExistFunc\"); load failed, return nullptr." << std::endl;
  }
  try
  {
    auto unknownFunc2 = lib.loadSymbol<printPoint_func>("notExistFunc");  // 加载失败抛出异常
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }
  std::cout << "---------testNotExistSymbol----------" << std::endl;
}