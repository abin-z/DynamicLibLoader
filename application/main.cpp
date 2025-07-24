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

struct box_t
{
  int id;         // box 编号
  char name[64];  // box 名称，固定长度字符串
  point_t min;    // 最小点
  point_t max;    // 最大点
};


using getPoint_func = point_t (*)();
using printPoint_func = void (*)(point_t);

// 函数指针类型定义
typedef void (*double_callback_t)(double x, double y, double z);  // 简单函数回调
typedef void (*point_callback_t)(point_t p);                      // 按值传递 point_t
typedef void (*box_callback_t)(box_t *p);                         // 指针传递 box_t
/// =========== 定义动态库中函数指针类型 end ===========

void func();
void testHasSymbol(const dll::dynamic_library &lib);
void testGetVariable(const dll::dynamic_library &lib);
void testGetVariable2(const dll::dynamic_library &lib);
void testNotExistSymbol(const dll::dynamic_library &lib);
void testNullLibrary();
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
    // ret = lib.invoke<int>("g_counter"); // ❌你想获取变量, 这个模板不允许, 请使用 get_variable
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

    testHasSymbol(lib);
    testGetVariable(lib);
    testGetVariable2(lib);
    
    testNullLibrary();
    testNotExistSymbol(lib);
  }
  catch (const std::exception &ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    return;
  }
}

void testNullLibrary()
{
  std::cout << "--------- testNullLibrary ----------" << std::endl;
  dll::dynamic_library lib;  // 默认构造函数创建一个空的动态库对象
  lib.unload();              // 显式释放资源, 但此时 handle_ 仍然是 nullptr
  if (!lib)
  {
    std::cout << "lib is not valid." << std::endl;
  }
  else
  {
    std::cout << "lib is valid." << std::endl;
  }
  try
  {
    lib.get<intAdd_func>("intAdd");  // 尝试获取一个函数符号, 会抛出异常
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }
  std::cout << "--------- testNullLibrary ----------" << std::endl;
}

void testHasSymbol(const dll::dynamic_library &lib)
{
  std::cout << "------ testHasSymbol ------" << std::endl;

  std::cout << "has_symbol(\"intAdd\"): " << lib.has_symbol("intAdd") << std::endl;
  std::cout << "has_symbol(\"g_version\"): " << lib.has_symbol("g_version") << std::endl;
  std::cout << "has_symbol(\"non_exist\"): " << lib.has_symbol("non_exist") << std::endl;
  std::cout << "has_symbol(\"floatAdd\"): " << lib.has_symbol("floatAdd") << std::endl;
  std::cout << "has_symbol(\"g_point\"): " << lib.has_symbol("g_point") << std::endl;
  std::cout << "has_symbol(\"g_point_ptr\"): " << lib.has_symbol("g_point_ptr") << std::endl;
  std::cout << "has_symbol(\"g_point_ptr\"): " << lib.has_symbol("g_point_ptr") << std::endl;
  std::cout << "has_symbol(\"g_point_ptr\"): " << lib.has_symbol("g_point_ptr") << std::endl;
  std::cout << "has_symbol(\"g_point_ptr\"): " << lib.has_symbol("g_point_ptr") << std::endl;
  std::cout << "has_symbol(\"g_point_ptr0\"): " << lib.has_symbol("g_point_ptr1") << std::endl;

  std::cout << "------ testHasSymbol ------" << std::endl;
}

/// @brief 测试获取动态库中的变量
void testGetVariable(const dll::dynamic_library &lib)
{
  std::cout << "--------- testGetVariable ----------" << std::endl;

  // 获取动态库版本号
  const char *version = lib.get_variable<const char *>("g_version");
  std::cout << "[get_variable] Dynamic Library Version: " << version << std::endl;

  // 获取动态库变量
  int counter = lib.get_variable<int>("g_counter");
  std::cout << "[get_variable] g_counter value = " << counter << std::endl;

  // 获取动态库指针变量
  int *counter_ptr = lib.get_variable<int *>("g_counter_ptr");
  std::cout << "[get_variable] g_counter_ptr value = " << *counter_ptr << std::endl;

  // 直接修改动态库中变量的值
  *counter_ptr = 101;

  // 获取动态库结构体变量
  point_t &point = lib.get_variable<point_t>("g_point");
  std::cout << "[get_variable] g_point value x = " << point.x << ", y = " << point.y << ", z = " << point.z
            << std::endl;
  // 直接修改动态库中变量的值, point是引用
  point.x = 8;

  // 获取动态库结构体指针变量
  point_t *point_ptr = lib.get_variable<point_t *>("g_point_ptr");
  std::cout << "[get_variable] g_point_ptr value x = " << point_ptr->x << ", y = " << point_ptr->y
            << ", z = " << point_ptr->z << std::endl;

  // // 错误用法：传函数类型
  // auto& f = lib.get_variable<void()>("my_function"); // ❌你想获取函数, 这个模板不允许, 请使用 get

  //////////////////////// try_get_variable
  // 获取动态库版本号
  const char **version2 = lib.try_get_variable<const char *>("g_version");
  std::cout << "[try_get_variable] Dynamic Library Version: " << *version2 << std::endl;

  // 获取动态库变量
  int *counter2 = lib.try_get_variable<int>("g_counter");
  std::cout << "[try_get_variable] g_counter value = " << *counter2 << std::endl;

  // 获取动态库结构体变量
  point_t *point2 = lib.try_get_variable<point_t>("g_point");
  std::cout << "[try_get_variable] g_point value x = " << point2->x << ", y = " << point2->y << ", z = " << point2->z
            << std::endl;

  std::cout << "--------- testGetVariable ----------" << std::endl;
}

void testGetVariable2(const dll::dynamic_library &lib)
{
  std::cout << "--------- testGetVariable2 ----------" << std::endl;

  // 获取动态库版本号, 注意需要是 const char **
  const char **version = lib.get<const char **>("g_version");
  const char *ver = lib.get_variable<const char *>("g_version");
  std::cout << "g_version ptr = " << static_cast<const void *>(version) << std::endl;
  if (version)
  {
    std::cout << "Dynamic Library Version: " << *version << ", " << ver << std::endl;
  }
  else
  {
    std::cout << "Failed to load version string" << std::endl;
  }

  // 获取动态库变量
  int *counter = lib.get<int *>("g_counter");
  std::cout << "g_counter addr = " << static_cast<void *>(counter) << ", value = " << (counter ? *counter : -1)
            << std::endl;
  // 获取动态库指针变量
  int **counter_ptr = lib.get<int **>("g_counter_ptr");
  std::cout << "g_counter_ptr addr = " << static_cast<void *>(counter_ptr)
            << ", value = " << (counter_ptr && *counter_ptr ? **counter_ptr : -1) << std::endl;

  // 获取动态库结构体变量
  point_t *point = lib.get<point_t>("g_point");
  std::cout << "g_point addr = " << static_cast<void *>(point);
  if (point) std::cout << ", value = (" << point->x << ", " << point->y << ", " << point->z << ")";
  std::cout << std::endl;

  // 获取动态库结构体指针变量
  point_t **point_ptr = lib.get<point_t **>("g_point_ptr");
  std::cout << "g_point_ptr addr = " << static_cast<void *>(point_ptr);
  if (point_ptr && *point_ptr)
    std::cout << ", value = (" << (*point_ptr)->x << ", " << (*point_ptr)->y << ", " << (*point_ptr)->z << ")";
  std::cout << std::endl;

  std::cout << "--------- testGetVariable2 ----------" << std::endl;
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