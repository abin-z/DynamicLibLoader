#pragma once
#include <fstream>
#include <iostream>
#include <string>

namespace Common
{
template <typename T>
inline T add(const T &a, const T &b)
{
  return a + b;
}

template <typename T>
inline void println(const T &value)
{
  std::cout << value << '\n';
}

template <typename T>
inline void print(const T &value)
{
  std::cout << value;
}

// 将字符串写入文件中，返回是否成功
inline bool writeStringToFile(const std::string &filename, const std::string &content)
{
  std::ofstream ofs(filename, std::ios::out | std::ios::app);  // 使用 std::ios::app 追加内容
  if (!ofs.is_open())
  {
    return false;
  }
  ofs << content;
  return ofs.good();  // 检查写入是否成功
}

}  // namespace Common
