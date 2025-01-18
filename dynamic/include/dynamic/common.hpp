#pragma once
#include <string>
#include <iostream>

namespace Common
{
  template <typename T>
  T add(const T &a, const T &b)
  {
    return a + b;
  }

  template <typename T>
  void println(const T &value)
  {
    std::cout << value << '\n';
  }

  template <typename T>
  void print(const T &value)
  {
    std::cout << value;
  }
  
} // namespace Common



