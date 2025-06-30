#include <dynamic/dynamic.h>

#include <dynamic/common.hpp>
#include <string>

// 版本号字符串，导出为只读全局变量
DLL_PUBLIC_API const char* g_version = "v1.2.3";

// 全局变量定义
DLL_PUBLIC_API int g_counter = 42;
DLL_PUBLIC_API int* g_counter_ptr = &g_counter;

// 结构体变量
DLL_PUBLIC_API point_t g_point = {9, 99, 999};
// 结构体指针
DLL_PUBLIC_API point_t* g_point_ptr = &g_point;

// 函数实现
DLL_PUBLIC_API void sayHello()
{
  Common::println("hello, I am from dynamicLib.");
}

DLL_PUBLIC_API int intAdd(int a, int b)
{
  return Common::add(a, b);
}
DLL_PUBLIC_API float floatAdd(float a, float b)
{
  return Common::add(a, b);
}
DLL_PUBLIC_API double doubleAdd(double a, double b)
{
  return Common::add(a, b);
}

DLL_PUBLIC_API point_t getPoint()
{
  return {1, 2, 3};
}

DLL_PUBLIC_API void printPoint(point_t arg)
{
  std::string str = "{x: " + std::to_string(arg.x) + " y: " + std::to_string(arg.y) + " z: " + std::to_string(arg.z) + "}";
  Common::print(str);
}
