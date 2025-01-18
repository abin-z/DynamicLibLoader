#pragma once

#include <stdexcept>
#include <string>

/*
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

    inline LibHandle loadLibrary(const std::string& path) {
        return LoadLibraryA(path.c_str());
    }

    inline void unloadLibrary(LibHandle handle) {
        FreeLibrary(handle);
    }

    template <typename Func>
    inline Func loadSymbol(LibHandle handle, const std::string& name) {
        return reinterpret_cast<Func>(GetProcAddress(handle, name.c_str()));
    }

    inline std::string getLastError() {
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
    using LibHandle = void*;

    inline LibHandle loadLibrary(const std::string& path) {
        return dlopen(path.c_str(), RTLD_LAZY);
    }

    inline void unloadLibrary(LibHandle handle) {
        if (handle) dlclose(handle);
    }

    template <typename Func>
    inline Func loadSymbol(LibHandle handle, const std::string& name) {
        dlerror(); // 清除之前的错误
        return reinterpret_cast<Func>(dlsym(handle, name.c_str()));
    }

    inline std::string getLastError() {
        const char* error = dlerror();
        return error ? std::string(error) : "Unknown error";
    }
#endif

/// @brief 动态库加载类, RAII实现方式
class DynamicLibrary {
public:
    explicit DynamicLibrary(const std::string& libPath) : handle(nullptr) {
        handle = loadLibrary(libPath);
        if (!handle) {
            throw std::runtime_error("Failed to load library: " + libPath + " - " + getLastError());
        }
    }

    ~DynamicLibrary() {
        unloadLibrary(handle);
    }

    template <typename Func>
    Func loadSymbol(const std::string& symbolName) {
        Func symbol = ::loadSymbol<Func>(handle, symbolName);
        if (!symbol) {
            throw std::runtime_error("Failed to load symbol: " + symbolName + " - " + getLastError());
        }
        return symbol;
    }

private:
    LibHandle handle; // 动态库句柄
};
