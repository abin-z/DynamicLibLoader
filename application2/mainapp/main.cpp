#include <iostream>

#include "dynamic.h"

// 回调函数示例
void on_double(double x, double y, double z)
{
  std::cout << "[callback double] (" << x << ", " << y << ", " << z << ")\n";
}

void on_point(point_t p)
{
  std::cout << "[callback point] (" << p.x << ", " << p.y << ", " << p.z << ")\n";
}

void on_box(box_t *b)
{
  std::cout << "[callback box] id=" << b->id << ", name=" << b->name << ", min=(" << b->min.x << "," << b->min.y << ","
            << b->min.z << ")" << ", max=(" << b->max.x << "," << b->max.y << "," << b->max.z << ")\n";
}

int main()
{
  std::cout << "This is application2, demonstrating implicit dynamic library loading." << std::endl;

  std::cout << "=== Test dynamic library ===" << std::endl;
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

  // 测试结构体返回
  point_t p = getPoint();
  std::cout << "getPoint(): (" << p.x << ", " << p.y << ", " << p.z << ")\n";
  printPoint(p);

  box_t b = getBox();

  // 转字符串
  char buf[256];
  box2String(b, buf, sizeof(buf));
  std::cout << "box2String(): " << buf << std::endl;

  point2String(&p, buf, sizeof(buf));
  std::cout << "point2String(): " << buf << std::endl;

  // 测试回调注册
  register_double_callback(on_double);
  register_point_callback(on_point);
  register_box_callback(on_box);

  std::cout << "Trigger callbacks...\n";
  trigger_callbacks(1);
  trigger_callbacks(2);
  trigger_callbacks(4);

  std::cout << "=== Done ===" << std::endl;

  return 0;
}