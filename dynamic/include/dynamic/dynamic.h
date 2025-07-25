#pragma once
#include "dll_export.h"

/*
 * 使用 extern "C" 的好处:
 * 避免 C++ 名称修饰, 使用 C 风格的函数接口, 确保了 符号命名一致性 和 函数调用约定一致性.
 * 使动态库对 C 和其他语言友好，增强了ABI兼容性。
 * 其他语言（如 C、Python、Java）调用这些接口时，必须使用一致的 ABI。
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

/// 可选：如果需要显式的内存对齐控制，可以使用 #pragma pack 或 __attribute__((packed))
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

// 函数指针类型定义
typedef void (*double_callback_t)(double x, double y, double z);  // 简单函数回调
typedef void (*point_callback_t)(point_t p);                      // 按值传递 point_t
typedef void (*box_callback_t)(box_t *p);                         // 指针传递 box_t

// 版本号字符串，导出为只读全局变量
DLL_PUBLIC_API extern const char *g_version;

// 导出全局变量
DLL_PUBLIC_API extern int g_counter;
DLL_PUBLIC_API extern int *g_counter_ptr;

// 结构体变量
DLL_PUBLIC_API extern point_t g_point;
// 结构体指针
DLL_PUBLIC_API extern point_t *g_point_ptr;

// 导出函数
DLL_PUBLIC_API void sayHello();

DLL_PUBLIC_API int intAdd(int a, int b);
DLL_PUBLIC_API float floatAdd(float a, float b);
DLL_PUBLIC_API double doubleAdd(double a, double b);

DLL_PUBLIC_API point_t getPoint();
DLL_PUBLIC_API void printPoint(point_t arg);

// 返回常量的Hello字符串
DLL_PUBLIC_API const char *getHelloString();
// 按值返回box_t对象
DLL_PUBLIC_API box_t getBox();

// 将参数arg转为字符串, 结果在buf中(buf的内存空间需要调用者分配), 字符串长度不超过max_size
// 参数 buf 必须是非空有效内存，长度至少为 max_size 字节
DLL_PUBLIC_API void box2String(box_t arg, char *buf, unsigned int max_size);
DLL_PUBLIC_API void point2String(point_t *arg, char *buf, unsigned int max_size);  // 参数arg为指针

// 1. 注册回调函数
DLL_PUBLIC_API void register_double_callback(double_callback_t cb);  // 函数 1
DLL_PUBLIC_API void register_point_callback(point_callback_t cb);    // 函数 2
DLL_PUBLIC_API void register_box_callback(box_callback_t cb);        // 函数 4

// 2. 调用注册的回调函数
// n = 1 表示调用 double_callback
// n = 2 表示调用 point_callback
// n = 4 表示调用 box_callback
DLL_PUBLIC_API void trigger_callbacks(int n);

#ifdef __cplusplus
}
#endif