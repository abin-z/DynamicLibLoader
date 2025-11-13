#include <iostream>

#include "dynamic.h"

int main()
{
  std::cout << "This is application2, demonstrating implicit dynamic library loading." << std::endl;
  // 打印库版本
  std::cout << "Library version: " << g_version << std::endl;

  // 测试全局变量
  std::cout << "g_counter = " << g_counter << std::endl;
  if (g_counter_ptr)
  {
    std::cout << "*g_counter_ptr = " << *g_counter_ptr << std::endl;
  }

  // 测试函数
  sayHello();

  std::cout << "intAdd(2,3) = " << intAdd(2, 3) << std::endl;
  std::cout << "floatAdd(1.5, 2.5) = " << floatAdd(1.5f, 2.5f) << std::endl;
  std::cout << "doubleAdd(3.14, 2.71) = " << doubleAdd(3.14, 2.71) << std::endl;

  return 0;
}