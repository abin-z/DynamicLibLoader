/**
 * @description: Cross-platform dynamic library explicit loading class (跨平台动态库显式加载类)
 * @author: abin
 * @date: 2025-01-19
 * @license: MIT
 *
 * @brief 跨平台动态库显式加载类 (类似于 Boost.DLL 的功能)
 * 为了使动态库加载实现跨平台，考虑 Windows 和 POSIX（如 Linux、macOS）平台的差异，采用条件编译。
 *
 * 主要特点：
 * 1. 动态库 API 的统一封装：
 *    - Windows 使用 `LoadLibraryA` 和 `GetProcAddress`；
 *    - POSIX 使用 `dlopen` 和 `dlsym`。
 * 2. 错误信息处理：
 *    - Windows 使用 `GetLastError` 并通过 `FormatMessageA` 获取详细错误信息；
 *    - POSIX 使用 `dlerror` 获取错误信息。
 * 3. 使用 `LibHandle` 统一管理句柄类型：
 *    - Windows 上句柄类型是 `HMODULE`；
 *    - POSIX 平台上句柄类型是 `void*`。
 * 4. 通过条件编译实现平台差异处理：
 *    - 使用 `#if defined(_WIN32) || defined(_WIN64)` 检测 Windows 平台，其他平台则使用 POSIX 实现。
 */

#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H

#include <stdexcept>
#include <string>
#include <type_traits>

namespace dll
{
namespace detail
{
// 定义平台相关的动态库 API
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
using LibHandle = HMODULE;

inline LibHandle loadLibrary(const std::string &path) noexcept
{
  return LoadLibraryA(path.c_str());
}

inline void unloadLibrary(LibHandle handle) noexcept
{
  if (handle)
  {
    FreeLibrary(handle);
  }
}

template <typename Func>
inline Func loadSymbol(LibHandle handle, const std::string &name) noexcept
{
  static_assert(std::is_pointer<Func>::value, "Func must be a pointer type");  // 确保 Func 是指针类型
  return reinterpret_cast<Func>(GetProcAddress(handle, name.c_str()));
}

inline std::string getLastError()
{
  DWORD error = GetLastError();
  LPVOID msgBuffer;
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                               error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, NULL);
  std::string message((LPSTR)msgBuffer, size);
  LocalFree(msgBuffer);
  return "Error Code: " + std::to_string(error) + " - " + message;
}

#else

#include <dlfcn.h>
using LibHandle = void *;

inline LibHandle loadLibrary(const std::string &path) noexcept
{
  return dlopen(path.c_str(), RTLD_LAZY);
}

inline void unloadLibrary(LibHandle handle) noexcept
{
  if (handle)
  {
    dlclose(handle);
  }
}

template <typename Func>
inline Func loadSymbol(LibHandle handle, const std::string &name) noexcept
{
  static_assert(std::is_pointer<Func>::value, "Func must be a pointer type");  // 确保 Func 是指针类型
  dlerror();                                                                   // 清除之前的错误
  return reinterpret_cast<Func>(dlsym(handle, name.c_str()));
}

inline std::string getLastError()
{
  const char *error = dlerror();
  return error ? std::string(error) : "Unknown error";
}
#endif

}  // namespace detail

using detail::LibHandle;

/// @brief 动态库加载类, 使用 RAII 管理动态库资源
class DynamicLibrary
{
 public:
  /**
   * @brief 构造函数, 加载指定路径的动态库
   * @param libPath 动态库路径
   * @throw std::runtime_error 如果加载失败, 则抛出异常
   */
  explicit DynamicLibrary(const std::string &libPath) : handle(nullptr)
  {
    handle = detail::loadLibrary(libPath);
    if (!handle)
    {
      throw std::runtime_error("Failed to load library: " + libPath + " - " + detail::getLastError());
    }
  }

  /// @brief 析构函数 - 自动卸载动态库
  ~DynamicLibrary()
  {
    detail::unloadLibrary(handle);
    handle = nullptr;
  }

  /**
   * @brief 加载符号, 加载失败抛出异常(异常导向式API)
   *
   * @tparam Func 函数指针类型, 用于指定要加载的符号对应的函数类型
   * @param symbolName 符号名称, 指定要加载的符号的名称
   * @return 返回加载的符号地址
   * @throw std::runtime_error 如果加载符号失败, 则抛出异常
   *
   * @note 该函数尝试加载动态库中的指定符号, 如果加载失败(如符号不存在),
   *       会抛出 `std::runtime_error` 异常, 异常信息中包含符号名称及加载错误信息.
   */
  template <typename Func>
  Func loadSymbol(const std::string &symbolName) const
  {
    Func symbol = detail::loadSymbol<Func>(handle, symbolName);
    if (!symbol)
    {
      throw std::runtime_error("Failed to load symbol: " + symbolName + " - " + detail::getLastError());
    }
    return symbol;
  }

  /**
   * @brief 尝试加载符号, 加载失败时不抛出异常, 而是返回 nullptr
   *
   * @tparam Func 函数指针类型, 用于指定要加载的符号对应的函数类型
   * @param symbolName 符号名称, 指定要加载的符号的名称
   * @return 返回加载的符号地址, 如果加载失败返回 nullptr
   *
   * @note 该函数不会抛出异常.如果符号加载失败, 返回值为 `nullptr`, 使用此函数时需检查返回值来确认加载是否成功.
   */
  template <typename Func>
  Func tryLoadSymbol(const std::string &symbolName) const noexcept
  {
    return detail::loadSymbol<Func>(handle, symbolName);
  }

  /// @brief 检查动态库是否已加载
  /// @return 如果已加载, 返回 true; 否则返回 false
  bool isLoaded() const noexcept
  {
    return handle != nullptr;
  }

  // 禁用拷贝语义, 防止拷贝可能导致的资源管理问题
  DynamicLibrary(const DynamicLibrary &) = delete;
  DynamicLibrary &operator=(const DynamicLibrary &) = delete;

  // 支持移动语义, 便于资源的安全转移 - 移动构造
  DynamicLibrary(DynamicLibrary &&other) noexcept : handle(other.handle)
  {
    other.handle = nullptr;
  }

  // 支持移动语义, 便于资源的安全转移 - 移动赋值
  DynamicLibrary &operator=(DynamicLibrary &&other) noexcept
  {
    if (this != &other)
    {
      if (handle)  // 卸载自身句柄
      {
        detail::unloadLibrary(handle);
      }
      handle = other.handle;
      other.handle = nullptr;
    }
    return *this;
  }

 private:
  LibHandle handle = nullptr;  // 动态库句柄
};

}  // namespace dll

#endif  // DYNAMIC_LIBRARY_H