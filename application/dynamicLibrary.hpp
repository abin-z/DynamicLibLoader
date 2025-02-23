/**
 * @description: Cross-platform dynamic lbirary explicit loading class (跨平台动态库显式加载类)
 * @author: abin
 * @date: 2025-01-19
 * @license: MIT
 */

#pragma once

#include <stdexcept>
#include <string>

/**
 * @brief 跨平台动态库显式加载类 (类似于Boost.DLL的功能)
 * 为了让动态库加载的实现跨平台，主要需要考虑 Windows 和 POSIX（如 Linux、macOS）平台的差异。以下是实现跨平台动态库加载的改进方式：
 *
 * 动态库 API 的统一封装：
 * Windows 使用 LoadLibraryA 和 GetProcAddress，而 POSIX 使用 dlopen 和 dlsym。
 * 定义统一的 loadLibrary、unloadLibrary 和 loadSymbol 函数。
 *
 * 错误信息处理：
 * Windows 使用 GetLastError 并通过 FormatMessageA 获取详细错误信息。
 * POSIX 使用 dlerror 获取错误信息。
 *
 * 使用 LibHandle 统一管理句柄类型：
 * 在 Windows 上，句柄类型是 HMODULE。
 * 在 POSIX 平台上，句柄类型是 void*。
 *
 * 条件编译：
 * 使用 #if defined(_WIN32) || defined(_WIN64) 检测 Windows 平台。
 * 其他平台默认使用 POSIX 的实现。
 */

// 定义平台相关的动态库 API
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
using LibHandle = HMODULE;

inline LibHandle loadLibrary(const std::string &path)
{
  return LoadLibraryA(path.c_str());
}

inline void unloadLibrary(LibHandle handle)
{
  if (handle)
  {
    FreeLibrary(handle);
    handle = nullptr;
  }
}

template <typename Func>
inline Func loadSymbol(LibHandle handle, const std::string &name)
{
  return reinterpret_cast<Func>(GetProcAddress(handle, name.c_str()));
}

inline std::string getLastError()
{
  DWORD error = GetLastError();
  LPVOID msgBuffer;
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, NULL);
  std::string message((LPSTR)msgBuffer, size);
  LocalFree(msgBuffer);
  return message;
}
#else
#include <dlfcn.h>
using LibHandle = void *;

inline LibHandle loadLibrary(const std::string &path)
{
  return dlopen(path.c_str(), RTLD_LAZY);
}

inline void unloadLibrary(LibHandle handle)
{
  if (handle)
  {
    dlclose(handle);
    handle = nullptr;
  }
}

template <typename Func>
inline Func loadSymbol(LibHandle handle, const std::string &name)
{
  dlerror(); // 清除之前的错误
  return reinterpret_cast<Func>(dlsym(handle, name.c_str()));
}

inline std::string getLastError()
{
  const char *error = dlerror();
  return error ? std::string(error) : "Unknown error";
}
#endif

/// @brief 动态库加载类，使用 RAII 管理动态库资源
class DynamicLibrary
{
public:
  /**
   * @brief 构造函数，加载指定路径的动态库
   * @param libPath 动态库路径
   * @throw std::runtime_error 如果加载失败，则抛出异常
   */
  explicit DynamicLibrary(const std::string &libPath) : handle(nullptr)
  {
    handle = loadLibrary(libPath);
    if (!handle)
    {
      throw std::runtime_error("Failed to load library: " + libPath + " - " + getLastError());
    }
  }

  /// @brief 析构函数 - 自动卸载动态库
  ~DynamicLibrary()
  {
    unloadLibrary(handle);
    handle = nullptr;
  }

  /**
   * @brief 加载符号
   * @tparam Func 函数指针类型
   * @param symbolName 符号名称
   * @return 符号地址
   * @throw std::runtime_error 如果加载符号失败，则抛出异常
   */
  template <typename Func>
  Func loadSymbol(const std::string &symbolName)
  {
    Func symbol = ::loadSymbol<Func>(handle, symbolName);
    if (!symbol)
    {
      throw std::runtime_error("Failed to load symbol: " + symbolName + " - " + getLastError());
    }
    return symbol;
  }

  /// @brief 检查动态库是否已加载
  /// @return 如果已加载，返回 true；否则返回 false
  bool isLoaded() const noexcept
  {
    return handle != nullptr;
  }

  // 禁用拷贝语义，防止拷贝可能导致的资源管理问题
  DynamicLibrary(const DynamicLibrary &) = delete;
  DynamicLibrary &operator=(const DynamicLibrary &) = delete;

  // 支持移动语义，便于资源的安全转移 - 移动构造
  DynamicLibrary(DynamicLibrary &&other) noexcept : handle(other.handle)
  {
    other.handle = nullptr;
  }

  // 支持移动语义，便于资源的安全转移 - 移动赋值
  DynamicLibrary &operator=(DynamicLibrary &&other) noexcept
  {
    if (this != &other)
    {
      if (handle) // 卸载自身句柄
      {
        unloadLibrary(handle);
      }
      handle = other.handle;
      other.handle = nullptr;
    }
    return *this;
  }

private:
  LibHandle handle = nullptr; // 动态库句柄
};
