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

DLL_PUBLIC_API void sayHello();

DLL_PUBLIC_API int intAdd(int a, int b);
DLL_PUBLIC_API float floatAdd(float a, float b);
DLL_PUBLIC_API double doubleAdd(double a, double b);

DLL_PUBLIC_API point_t getPoint();
DLL_PUBLIC_API void printPoint(point_t arg);

#ifdef __cplusplus
}
#endif