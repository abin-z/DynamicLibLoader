#include <dynamic/dynamic.h>

#include <dynamic/common.hpp>
#include <string>

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
