#pragma once

/*
 * 推荐在CMakeList.txt中定义宏 DLL_PUBLIC_EXPORTS，而不必在源文件中手动定义它。
 * 这样可以确保在编译动态库时自动设置该宏，从而在库内部导出符号.(win系统上需要)
 *
 * 定义 DLL_PUBLIC_EXPORTS 宏，表示这是一个导出库
 * target_compile_definitions(targetname PRIVATE DLL_PUBLIC_EXPORTS)
 * 
 */

#ifdef _WIN32
// Windows 系统
#ifdef DLL_PUBLIC_EXPORTS
#define DLL_PUBLIC_API __declspec(dllexport) // 导出符号
#else
#define DLL_PUBLIC_API __declspec(dllimport) // 导入符号
#endif
#elif defined(__GNUC__) || defined(__clang__)
// GCC 或 Clang 编译器
#define DLL_PUBLIC_API __attribute__((visibility("default"))) // 使用默认可见性，适用于Linux、macOS等
#else
// 默认情况，假定没有特殊的导入导出要求
#define DLL_PUBLIC_API
#endif
