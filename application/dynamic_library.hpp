/*********************************************************************************************************
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * @file: dynamic_library.hpp
 * @version: v0.9.0
 * @description: Cross-platform dynamic library explicit loader
 *    - This class provides a cross-platform way to explicitly load dynamic libraries.
 *    - It uses conditional compilation to handle differences between Windows and POSIX platforms (such as Linux and macOS).
 *
 * Key Features:
 * 1. Unified Dynamic Library API:
 *    - Uses `LoadLibraryA` and `GetProcAddress` on Windows.
 *    - Uses `dlopen` and `dlsym` on POSIX-compliant systems.
 * 2. Error Handling:
 *    - On Windows, retrieves detailed error messages via `GetLastError` and `FormatMessageA`.
 *    - On POSIX, uses `dlerror` for error reporting.
 * 3. Unified Handle Abstraction:
 *    - Uses `HMODULE` as the library handle on Windows.
 *    - Uses `void*` on POSIX systems.
 * 4. Platform Detection via Conditional Compilation:
 *    - Uses `#if defined(_WIN32) || defined(_WIN64)` to detect Windows.
 *    - Defaults to POSIX implementation on other platforms.
 *
 * @author: abin
 * @date: 2025-01-19
 * @license: MIT
 * @repository: https://github.com/abin-z/DynamicLibLoader
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *********************************************************************************************************/

#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H

#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace dll
{
namespace detail
{
/// @brief 通用函数指针类型适配器
template <typename T>
struct function_pointer_traits
{
  using type = typename std::remove_pointer<typename std::remove_reference<T>::type>::type *;
};

/// @brief 通用函数指针类型别名
template <typename T>
using function_pointer_t = typename function_pointer_traits<T>::type;

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
inline function_pointer_t<Func> loadSymbol(LibHandle handle, const std::string &name) noexcept
{
  return reinterpret_cast<function_pointer_t<Func>>(GetProcAddress(handle, name.c_str()));
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
inline function_pointer_t<Func> loadSymbol(LibHandle handle, const std::string &name) noexcept
{
  dlerror();  // 清除之前的错误
  return reinterpret_cast<function_pointer_t<Func>>(dlsym(handle, name.c_str()));
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
  template <typename T>
  using function_pointer_t = typename detail::function_pointer_t<T>;

 public:
  /**
   * @brief 构造函数, 加载指定路径的动态库
   * @param libPath 动态库路径
   * @throw std::runtime_error 如果加载失败, 则抛出异常
   */
  explicit DynamicLibrary(const std::string &libPath) : handle_(nullptr)
  {
    handle_ = detail::loadLibrary(libPath);
    if (!handle_)
    {
      throw std::runtime_error("Failed to load library: " + libPath + " - " + detail::getLastError());
    }
  }

  /// @brief 析构函数 - 自动卸载动态库
  ~DynamicLibrary()
  {
    if (handle_)  // 只有在 handle 非 nullptr 时才卸载
    {
      detail::unloadLibrary(handle_);
      handle_ = nullptr;
    }
  }

  // 禁用拷贝语义, 防止拷贝可能导致的资源管理问题
  DynamicLibrary(const DynamicLibrary &) = delete;
  DynamicLibrary &operator=(const DynamicLibrary &) = delete;

  // 支持移动语义, 便于资源的安全转移 - 移动构造
  DynamicLibrary(DynamicLibrary &&other) noexcept : handle_(other.handle_), cache_(std::move(other.cache_))
  {
    other.handle_ = nullptr;
    other.cache_.clear();
  }

  // 支持移动语义, 便于资源的安全转移 - 移动赋值
  DynamicLibrary &operator=(DynamicLibrary &&rhs) noexcept
  {
    // 常规实现方式
    // if (this != &rhs)
    // {
    //   if (handle_)  // 卸载自身句柄
    //   {
    //     detail::unloadLibrary(handle_);
    //   }
    //   handle_ = rhs.handle_;
    //   cache_ = std::move(rhs.cache_);
    //   rhs.handle_ = nullptr;
    //   rhs.cache_.clear();
    // }

    // copy-and-swap方式
    DynamicLibrary tmp(std::move(rhs));  // 用移动构造拿走 rhs 的资源
    swap(*this, tmp);                    // 安全交换，老对象资源由 tmp 析构释放
    return *this;                        // tmp 析构，释放旧资源
  }

  // swap 函数：强异常安全，供移动赋值使用
  friend void swap(DynamicLibrary &lhs, DynamicLibrary &rhs) noexcept
  {
    using std::swap;
    swap(lhs.handle_, rhs.handle_);
    swap(lhs.cache_, rhs.cache_);
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
  function_pointer_t<Func> loadSymbol(const std::string &symbolName) const
  {
    auto symbol = detail::loadSymbol<Func>(handle_, symbolName);
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
  function_pointer_t<Func> tryLoadSymbol(const std::string &symbolName) const noexcept
  {
    return detail::loadSymbol<Func>(handle_, symbolName);
  }

  /**
   * @brief 调用动态库中的符号, 支持参数转发
   *
   * @tparam Func 函数指针类型, 用于指定要调用的符号对应的函数类型
   * @tparam Args 可变参数模板, 用于指定传递给函数的参数类型
   * @param symbolName 符号名称, 指定要调用的符号的名称
   * @param args 可变参数, 传递给函数的参数
   * @return 返回函数调用的结果
   * @throw `std::runtime_error` 如果加载符号失败, 则抛出异常
   *
   * @note 该函数会尝试加载并调用动态库中的指定符号.如果符号不存在或加载失败,
   *       会抛出 `std::runtime_error` 异常.使用此函数时需确保符号名称正确.
   */
  template <typename Func, typename... Args>
  auto invokeSymbol(const std::string &symbolName, Args... args) const
    -> decltype(std::declval<Func>()(std::forward<Args>(args)...))
  {
    auto it = cache_.find(symbolName);
    if (it == cache_.end())
    {
      auto symbol = loadSymbol<Func>(symbolName);                               // 加载符号
      it = cache_.emplace(symbolName, reinterpret_cast<void *>(symbol)).first;  // 缓存符号
    }
    return reinterpret_cast<function_pointer_t<Func>>(it->second)(std::forward<Args>(args)...);
  }

  /// @brief 检查动态库是否已加载
  /// @return 如果已加载, 返回 true; 否则返回 false
  bool isLoaded() const noexcept
  {
    return handle_ != nullptr;
  }

  /// @brief 获取动态库底层原生句柄 (Windows 的 `HMODULE` 或 POSIX 的 `void*`)
  /// @return 底层原生句柄
  /// @note
  /// - 返回的句柄是操作系统的原生句柄, 直接操作时请小心.确保 `DynamicLibrary` 对象的生命周期有效,
  ///   避免在销毁前手动释放或操作句柄.不正确的手动操作句柄可能导致资源泄露或不正确的资源管理(破坏RAII机制).
  LibHandle nativeHandle() const noexcept
  {
    return handle_;
  }

 private:
  LibHandle handle_ = nullptr;                             // 动态库句柄
  mutable std::unordered_map<std::string, void *> cache_;  // 符号缓存
};

}  // namespace dll

#endif  // DYNAMIC_LIBRARY_H