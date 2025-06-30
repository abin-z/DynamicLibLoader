/*********************************************************************************************************
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * @file: dynamic_library.hpp
 * @version: v0.9.2
 * @description: Cross-platform dynamic library explicit loader
 *    - Provides a cross-platform way to load dynamic libraries.
 *    - Uses conditional compilation for platform-specific handling (Windows vs POSIX: Linux/macOS).
 *
 * Key Features:
 *    - Cross-platform support: Compatible with Windows and POSIX (Linux/macOS).
 *    - RAII Resource Management: Loads the library on construction and unloads it on destruction.
 *    - Error Handling: Throws detailed `std::runtime_error` exceptions with platform-specific error messages
 *      when failing to load the library or symbols.
 *    - Symbol Caching: `invoke()` supports symbol caching for improved efficiency.
 *    - Cached and Uncached Interfaces: Use `invoke()` (cached) or `invoke_uncached()` (non-cached).
 *    - No Dependencies: Relies solely on the standard library.
 *
 * @author: abin
 * @date: 2025-01-19
 * @license: MIT
 * @repository: https://github.com/abin-z/DynamicLibLoader
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *********************************************************************************************************/

#pragma once  // 非标准但广泛支持的写法
#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H

#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace dll
{
namespace detail
{
/// @brief 通用符号（变量或函数）指针类型适配器
///        - 用于获取适合 dlsym/GetProcAddress 转换的原始指针类型
///        - 支持函数类型、变量类型，自动剥离引用和一级指针，再加上一级指针
template <typename T>
struct symbol_pointer_traits
{
  using type =
    typename std::add_pointer<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::type;
};

/// @brief 通用符号指针类型别名
///        - 给定类型 T，生成适合用于动态库符号加载的指针类型
template <typename T>
using symbol_pointer_t = typename symbol_pointer_traits<T>::type;

// 定义平台相关的动态库 API
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
using library_handle = HMODULE;

inline library_handle load_library(const std::string &path) noexcept
{
  return LoadLibraryA(path.c_str());
}

inline void unload_library(library_handle handle) noexcept
{
  if (handle)
  {
    FreeLibrary(handle);
  }
}

template <typename F>
inline symbol_pointer_t<F> load_symbol(library_handle handle, const std::string &name) noexcept
{
  return reinterpret_cast<symbol_pointer_t<F>>(GetProcAddress(handle, name.c_str()));
}

inline std::string get_last_error()
{
  DWORD error = GetLastError();
  if (error == 0) return "No error";

  LPVOID msgBuffer = nullptr;
  size_t size =
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                   error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, NULL);
  std::string message;
  if (size && msgBuffer)
  {
    message.assign((LPSTR)msgBuffer, size);
    LocalFree(msgBuffer);
  }
  else
  {
    message = "Unknown error";
  }
  return "Error Code: " + std::to_string(error) + " - " + message;
}

#else

#include <dlfcn.h>
using library_handle = void *;

inline library_handle load_library(const std::string &path) noexcept
{
  return dlopen(path.c_str(), RTLD_LAZY);
}

inline void unload_library(library_handle handle) noexcept
{
  if (handle)
  {
    dlclose(handle);
  }
}

template <typename F>
inline symbol_pointer_t<F> load_symbol(library_handle handle, const std::string &name) noexcept
{
  dlerror();  // 清除之前的错误
  return reinterpret_cast<symbol_pointer_t<F>>(dlsym(handle, name.c_str()));
}

inline std::string get_last_error()
{
  const char *error = dlerror();
  return error ? std::string(error) : "Unknown error";
}
#endif

}  // namespace detail

using detail::library_handle;

/// @brief 动态库加载类, 使用 RAII 管理动态库资源
class dynamic_library
{
  template <typename T>
  using symbol_pointer_t = typename detail::symbol_pointer_t<T>;

 public:
  /**
   * @brief 构造函数, 加载指定路径的动态库
   * @param libPath 动态库路径
   * @throw std::runtime_error 如果加载失败, 则抛出异常
   */
  explicit dynamic_library(const std::string &libPath) : handle_(nullptr)
  {
    load_handle(libPath);
  }

  /// @brief 析构函数 - 自动卸载动态库
  ~dynamic_library()
  {
    unload_handle();
  }

  // 禁用拷贝语义, 防止拷贝可能导致的资源管理问题
  dynamic_library(const dynamic_library &) = delete;
  dynamic_library &operator=(const dynamic_library &) = delete;

  // 支持移动语义, 便于资源的安全转移 - 移动构造
  dynamic_library(dynamic_library &&other) noexcept : handle_(other.handle_), cache_(std::move(other.cache_))
  {
    other.handle_ = nullptr;
  }

  // 支持移动语义, 便于资源的安全转移 - 移动赋值
  dynamic_library &operator=(dynamic_library &&rhs) noexcept
  {
    // 常规实现方式
    // if (this != &rhs)
    // {
    //   if (handle_)  // 卸载自身句柄
    //   {
    //     detail::unload_library(handle_);
    //   }
    //   handle_ = rhs.handle_;
    //   cache_ = std::move(rhs.cache_);
    //   rhs.handle_ = nullptr;
    //   rhs.cache_.clear();
    // }

    // move-and-swap方式(自赋值安全, 异常安全)
    dynamic_library tmp(std::move(rhs));  // tmp 用移动构造拿走 rhs 的资源
    swap(*this, tmp);                     // 当前对象和 tmp 交换，tmp 拿到旧资源
    return *this;                         // tmp 析构，释放旧资源
  }

  // swap 函数：强异常安全，供移动赋值使用
  friend void swap(dynamic_library &lhs, dynamic_library &rhs) noexcept
  {
    using std::swap;
    swap(lhs.handle_, rhs.handle_);
    swap(lhs.cache_, rhs.cache_);
  }

  /// @brief 检查动态库是否已加载
  explicit operator bool() const noexcept
  {
    return valid();
  }

  /**
   * @brief 加载符号, 加载失败抛出异常(异常导向式API)
   *
   * @tparam F 函数指针类型, 用于指定要加载的符号对应的函数类型
   * @param symbol_name 符号名称, 指定要加载的符号的名称
   * @return 返回加载的符号地址
   * @throw std::runtime_error 如果加载符号失败, 则抛出异常
   *
   * @note 该函数尝试加载动态库中的指定符号, 如果加载失败(如符号不存在),
   *       会抛出 `std::runtime_error` 异常, 异常信息中包含符号名称及加载错误信息.
   */
  template <typename F>
  symbol_pointer_t<F> get(const std::string &symbol_name) const
  {
    auto symbol = detail::load_symbol<F>(handle_, symbol_name);
    if (!symbol)
    {
      throw std::runtime_error("Failed to load symbol: " + symbol_name + " - " + detail::get_last_error());
    }
    return symbol;
  }

  /**
   * @brief 尝试加载符号, 加载失败时不抛出异常, 而是返回 nullptr
   *
   * @tparam F 函数指针类型, 用于指定要加载的符号对应的函数类型
   * @param symbol_name 符号名称, 指定要加载的符号的名称
   * @return 返回加载的符号地址, 如果加载失败返回 nullptr
   *
   * @note 该函数不会抛出异常.如果符号加载失败, 返回值为 `nullptr`, 使用此函数时需检查返回值来确认加载是否成功.
   */
  template <typename F>
  symbol_pointer_t<F> try_get(const std::string &symbol_name) const noexcept
  {
    return detail::load_symbol<F>(handle_, symbol_name);
  }

  /**
   * @brief 调用动态库中的符号, 支持参数转发(调用后会缓存函数)
   *
   * @tparam F 函数指针类型, 用于指定要调用的符号对应的函数类型
   * @tparam Args 可变参数模板, 用于指定传递给函数的参数类型
   * @param symbol_name 符号名称, 指定要调用的符号的名称
   * @param args 可变参数, 传递给函数的参数
   * @return 返回函数调用的结果
   * @throw `std::runtime_error` 如果加载符号失败, 则抛出异常
   *
   * @note 该函数会尝试加载并调用动态库中的指定符号.如果符号不存在或加载失败,
   *       会抛出 `std::runtime_error` 异常.使用此函数时需确保符号名称正确.
   */
  template <typename F, typename... Args>
  auto invoke(const std::string &symbol_name, Args... args) const
    -> decltype(std::declval<F>()(std::forward<Args>(args)...))
  {
    using func_ptr = symbol_pointer_t<F>;
    func_ptr symbol = nullptr;
    {
      std::lock_guard<std::mutex> lock(mtx_);
      auto it = cache_.find(symbol_name);  // 查找缓存
      if (it != cache_.end())              // 找到了符号
      {
        symbol = reinterpret_cast<func_ptr>(it->second);
      }
    }
    if (!symbol)  // 未找到符号
    {
      symbol = get<F>(symbol_name);  // 加载符号, 加载失败抛异常
      {
        std::lock_guard<std::mutex> lock(mtx_);
        cache_.emplace(symbol_name, reinterpret_cast<void *>(symbol));  // 添加缓存
      }
    }
    return symbol(std::forward<Args>(args)...);  // 调用函数
  }

  /**
   * @brief 调用动态库中的符号, 支持参数转发(不使用缓存)
   *
   * @tparam F 函数指针类型, 用于指定要调用的符号对应的函数类型
   * @tparam Args 可变参数模板, 用于指定传递给函数的参数类型
   * @param symbol_name 符号名称, 指定要调用的符号的名称
   * @param args 可变参数, 传递给函数的参数
   * @return 返回函数调用的结果
   * @throw `std::runtime_error` 如果加载符号失败, 则抛出异常
   *
   * @note 该函数会尝试加载并调用动态库中的指定符号.如果符号不存在或加载失败,
   *       会抛出 `std::runtime_error` 异常.使用此函数时需确保符号名称正确.
   */
  template <typename F, typename... Args>
  auto invoke_uncached(const std::string &symbol_name, Args... args) const
    -> decltype(std::declval<F>()(std::forward<Args>(args)...))
  {
    return get<F>(symbol_name)(std::forward<Args>(args)...);  // 直接调用函数
  }

  /// @brief 检查动态库是否已加载
  /// @return 如果已加载, 返回 true; 否则返回 false
  bool valid() const noexcept
  {
    return handle_ != nullptr;
  }

  /// @brief 卸载原来的动态库, 重新加载动态库, 加载失败抛出异常`std::runtime_error`
  /// @param libPath 新的动态库路径
  void reload(const std::string &libPath)
  {
    unload();
    load_handle(libPath);
  }

  /// @brief 显式释放动态库资源(提前释放)
  void unload() noexcept
  {
    unload_handle();
    clear_cache();
  }

  /// @brief 获取动态库底层原生句柄 (Windows 的 `HMODULE` 或 POSIX 的 `void*`)
  /// @return 底层原生句柄
  /// @note
  /// - 返回的句柄是操作系统的原生句柄, 直接操作时请小心.确保 `dynamic_library` 对象的生命周期有效,
  ///   避免在销毁前手动释放或操作句柄.不正确的手动操作句柄可能导致资源泄露或不正确的资源管理(破坏RAII机制).
  library_handle native_handle() const noexcept
  {
    return handle_;
  }

 private:
  /// @brief 只加载动态库, 加载失败抛出异常`std::runtime_error`
  /// @param libPath 动态库路径
  void load_handle(const std::string &libPath)
  {
    handle_ = detail::load_library(libPath);
    if (!handle_)
    {
      throw std::runtime_error("Failed to load library: " + libPath + " - " + detail::get_last_error());
    }
  }

  /// @brief 只是卸载动态库
  void unload_handle() noexcept
  {
    if (handle_)  // 只有在 handle 非 nullptr 时才卸载
    {
      detail::unload_library(handle_);
      handle_ = nullptr;
    }
  }

  /// @brief 清除符号表缓存
  void clear_cache() const noexcept
  {
    std::lock_guard<std::mutex> lock(mtx_);
    cache_.clear();
  }

 private:
  library_handle handle_ = nullptr;                        // 动态库句柄
  mutable std::unordered_map<std::string, void *> cache_;  // 符号缓存
  mutable std::mutex mtx_;                                 // 互斥锁, 保护符号缓存线程安全
};

}  // namespace dll

#endif  // DYNAMIC_LIBRARY_H