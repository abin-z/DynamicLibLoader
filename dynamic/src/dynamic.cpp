#include <dynamic/dynamic.h>

#include <cstdio>   // snprintf
#include <cstring>  // memset
#include <dynamic/common.hpp>
#include <string>

// 版本号字符串，导出为只读全局变量
DLL_PUBLIC_API const char *g_version = "v1.2.3";

// 全局变量定义
DLL_PUBLIC_API int g_counter = 42;
DLL_PUBLIC_API int *g_counter_ptr = &g_counter;

// 结构体变量
DLL_PUBLIC_API point_t g_point = {9, 99, 999};
// 结构体指针
DLL_PUBLIC_API point_t *g_point_ptr = &g_point;

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
  std::string str =
    "{x: " + std::to_string(arg.x) + " y: " + std::to_string(arg.y) + " z: " + std::to_string(arg.z) + "}";
  Common::print(str);
}

// 返回常量的Hello字符串
DLL_PUBLIC_API const char *getHelloString()
{
  static const char msg[] = "Hello World from DynamicLib!";
  return msg;
}
// 按值返回box_t对象
DLL_PUBLIC_API box_t getBox()
{
  box_t b;
  b.id = 42;
  strncpy(b.name, "Box Object id = 42", sizeof(b.name));
  b.name[sizeof(b.name) - 1] = '\0';  // 确保字符串结尾安全
  b.min.x = 123;
  b.min.y = 1234;
  b.min.z = 12345;
  b.max.x = 777;
  b.max.y = 888;
  b.max.z = 999;
  return b;
}

// 将 box_t 转成字符串形式，写入 buf，长度不超过 max_size（包含末尾 \0）
DLL_PUBLIC_API void box2String(box_t arg, char *buf, unsigned int max_size)
{
  if (!buf || max_size == 0) return;

  // 格式化示例字符串：
  // "box_t { id=42, name='Box Object id = 42', min=(123.000000,1234.000000,12345.000000),
  // max=(777.000000,888.000000,999.000000) }"
  int ret = snprintf(buf, max_size, "box_t { id=%d, name='%s', min=(%.6f,%.6f,%.6f), max=(%.6f,%.6f,%.6f) }", arg.id,
                     arg.name, arg.min.x, arg.min.y, arg.min.z, arg.max.x, arg.max.y, arg.max.z);

  // snprintf 会自动保证以 '\0' 结尾，若字符串超长，则会截断输出
  // ret 返回写入的字符数（不包含 '\0'），可以根据需要检查是否超出 max_size
  (void)ret;  // 如果不需要额外处理，可以忽略
}

// 将 point_t 指针指向的数据转成字符串，写入 buf，长度不超过 max_size（包含末尾 \0）
DLL_PUBLIC_API void point2String(point_t *arg, char *buf, unsigned int max_size)
{
  if (!buf || max_size == 0 || !arg) return;

  // 格式化示例字符串：
  // "point_t { x=123.000000, y=456.000000, z=789.000000 }"
  int ret = snprintf(buf, max_size, "point_t { x=%.6f, y=%.6f, z=%.6f }", arg->x, arg->y, arg->z);

  (void)ret;
}

// 静态全局变量存储回调函数指针，初始为 nullptr
static double_callback_t g_double_cb = nullptr;
static point_callback_t g_point_cb = nullptr;
static box_callback_t g_box_cb = nullptr;

// 1. 注册回调函数
void register_double_callback(double_callback_t cb)
{
  g_double_cb = cb;
}
void register_point_callback(point_callback_t cb)
{
  g_point_cb = cb;
}
void register_box_callback(box_callback_t cb)
{
  g_box_cb = cb;
}

// 2. 调用注册的回调函数
// 根据参数 n 的位标志按位判断，触发对应类型的回调函数（如果已注册）
// n 的每一位表示是否触发某个类型的回调：
//   bit 0（值为 1）：触发 double 回调（g_double_cb）
//   bit 1（值为 2）：触发 point 回调（g_point_cb）
//   bit 2（值为 4）：触发 box 回调（g_box_cb）
DLL_PUBLIC_API void trigger_callbacks(int n)
{
  if ((n & 1) && g_double_cb)
  {
    g_double_cb(1.1, 2.2, 3.3);
  }

  if ((n & 2) && g_point_cb)
  {
    point_t p = {10.0, 20.0, 30.0};
    g_point_cb(p);
  }

  if ((n & 4) && g_box_cb)
  {
    static box_t box_example = {100, "Example Box With Callback", {0.1, 0.2, 0.3}, {9.9, 8.8, 7.7}};
    g_box_cb(&box_example);
  }
}
