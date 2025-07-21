#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "bp3k.h"  // <bp3k.h> if installed

// Using ibitpacker for value-type deduction (might as well)
using ipack6x32 = bp3k::ibitpacker<6, 32>;

// Fetching the deduced value type
using T = typename ipack6x32::value_type;

inline void print_elements(const ipack6x32& bp) noexcept {
  std::printf("%hhd", bp[0]);

  for (std::size_t i = 1; i < 32; ++i) std::printf(" %hhd", bp[i]);

  std::putc('\n', stdout);
}

int main(void) {
  // Default constructor
  ipack6x32 bp1;

  std::printf("bp1: ");
  print_elements(bp1);

  // Fill constructor (initializes all 32 elements to -7)
  ipack6x32 bp2(-7);

  std::printf("bp2: ");
  print_elements(bp2);

  // Value range (static constants)
  std::printf("value_min: %hhd\n", (T)ipack6x32::value_min);
  std::printf("value_max: %hhd\n", (T)ipack6x32::value_max);

  // Data access

  bp1[7] = ipack6x32::value_max - 1;
  std::printf("bp1[7]: %hhd\n", (T)bp1[7]);

  bp2.at(7) = ipack6x32::value_max - 2;
  std::printf("bp2.at(7): %hhd\n", (T)bp2.at(7));

  bp1.front() = ipack6x32::value_max - 3;
  std::printf("bp1.front(): %hhd\n", (T)bp1.front());

  bp2.back() = ipack6x32::value_max - 4;
  std::printf("bp2.back(): %hhd\n", (T)bp2.back());

  // Operations

  bp2.fill(ipack6x32::value_min);
  bp1.swap(bp2);

  // Lexicographic comparison

  bp2 = bp1;                              // both filled with value_min
  bp2.back() = ipack6x32::value_min + 1;  // make bp2 greater

  std::cout << "bp1 == bp2: " << std::boolalpha << (bp1 == bp2) << std::endl;
  std::cout << "bp1 < bp2: " << std::boolalpha << (bp1 < bp2) << std::endl;
  std::cout << "bp1 <= bp2: " << std::boolalpha << (bp1 <= bp2) << std::endl;
  std::cout << "bp1 > bp2: " << std::boolalpha << (bp1 > bp2) << std::endl;
  std::cout << "bp1 >= bp2: " << std::boolalpha << (bp1 >= bp2) << std::endl;

  return 0;
}
