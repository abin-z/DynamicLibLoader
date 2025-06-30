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
void testGetVariable(const dll::dynamic_library &lib);
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
    dll::dynamic_library lib0(libPath);
    dll::dynamic_library lib1(libPath);
    // dll::dynamic_library lib = lib0;  错误: 禁止拷贝构造
    // lib0 = lib1;                      错误: 禁止拷贝赋值

    lib0 = std::move(lib1);                     // 支持移动赋值
    dll::dynamic_library lib(std::move(lib0));  // 支持移动构造

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
    ret = lib.invoke<int (*)(int, int)>("intAdd", 4, 5);
    ret = lib.invoke<int (*)(int, int)>("intAdd", 5, 6);
    ret = lib.invoke<int (&)(int, int)>("intAdd", 6, 7);
    ret = lib.invoke<int (&)(int, int)>("intAdd", 7, 8);
    ret = lib.invoke<int (&&)(int, int)>("intAdd", 8, 9);
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

    testGetVariable(lib);

    testNotExistSymbol(lib);
  }
  catch (const std::exception &ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    return;
  }
}

/// @brief 测试获取动态库中的变量
void testGetVariable(const dll::dynamic_library &lib)
{
  std::cout << "--------- testGetVariable ----------" << std::endl;

  int *counter = lib.get<int *>("g_counter");
  std::cout << "g_counter addr = " << static_cast<void *>(counter) << ", value = " << (counter ? *counter : -1)
            << std::endl;

  int **counter_ptr = lib.get<int **>("g_counter_ptr");
  std::cout << "g_counter_ptr addr = " << static_cast<void *>(counter_ptr)
            << ", value = " << (counter_ptr && *counter_ptr ? **counter_ptr : -1) << std::endl;

  point_t *point = lib.get<point_t *>("g_point");
  std::cout << "g_point addr = " << static_cast<void *>(point);
  if (point) std::cout << ", value = (" << point->x << ", " << point->y << ", " << point->z << ")";
  std::cout << std::endl;

  point_t **point_ptr = lib.get<point_t **>("g_point_ptr");
  std::cout << "g_point_ptr addr = " << static_cast<void *>(point_ptr);
  if (point_ptr && *point_ptr)
    std::cout << ", value = (" << (*point_ptr)->x << ", " << (*point_ptr)->y << ", " << (*point_ptr)->z << ")";
  std::cout << std::endl;

  std::cout << "--------- testGetVariable ----------" << std::endl;
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
    // 正常调用: symbol是对的, 函数签名也正常
    double ret = lib.invoke<double(double, double)>("doubleAdd", 1.5, 3.0);  
    std::cout << "lib.invoke ret = " << ret << std::endl;
    // 未定义行为: symbol是对的,但是函数签名不一致
    double ret2 = lib.invoke<double(double, double, double)>("doubleAdd", 1.5, 3.0, 1.0);  
    std::cout << "[UB] lib.invoke ret2 = " << ret2 << std::endl;
    // 未定义行为: symbol是对的,但是函数签名不一致
    double ret3 = lib.invoke<double()>("doubleAdd");  
    std::cout << "[UB] lib.invoke ret3 = " << ret3 << std::endl;
  }
  catch (const std::exception &e)
  {
    std::cerr << "invoke error: " << e.what() << '\n';
  }

  std::cout << "---------testNotExistSymbol----------" << std::endl;
}