// main.cpp
// This program calculates the max of 3 numbers or strings (lexicographically).

#include <iostream>

template<typename T>
T max3(T first, T second, T third) {
  return (first > second ? (first > third) ? first : third : second);
}


int main() {

  std::cout << max3<double>(3.14, 2.72, 1.62) << '\n';
  std::cout << max3<std::string>("pi", "epsilon", "phi") << '\n';

  return 0;
}
