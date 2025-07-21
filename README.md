# BITPACKER3000
BP3K is a metaprogramming library that values memory conservation, random access, and freedom from exceptions.

## Purpose & Benefits
BP3K's primary templated type, `bp3k::bitpacker<T, W, N>`, provides functionality similar to `std::array<T, N>`, but stores values with arbitrary bit widths. The I/O value type, `T`, can be any signed, unsigned, or enumeration type.

## Getting Started
_Note: BP3K requires C++17 or greater._

### Installation
First clone the repository:
```bash
git clone https://github.com/croqueue/BP3K.git
cd BP3K
```

To install the library in your project, simply copy the single header file (`include/bp3k.h`) to your project's `include/` directory:
```bash
cp include/bp3k.h $YOUR_PROJECT_DIRECTORY
```

Alternatively, you can install the library system wide:
```bash
cp include/bp3k.h /usr/local/include/  # or a different include directory
```

### Unit Tests
_Note: BP3K's unit tests require CMake to build._

```bash
# Initialize the build
cmake -S . -B build
# Compile the unit tests
cmake --build build
# Run the tests
ctest --test-dir build/tests
```

The build may take a bit (as well as running the currently 1,496 tests).

### Documentation
_Note: BP3K's documentation requires Doxygen to render locally._  
  
```bash
# Render the dox
doxygen
# View documentation locally
xdg-open docs/html/index.html
```

## Usage Examples
BP3K exposes the following template types:
- `bp3k::ibitpacker<std::size_t W, std::size_t N>`
  - Signed-integer packer with type deduction based on `W` (narrowest adequate type)
- `bp3k::ubitpacker<std::size_t W, std::size_t N>`
  - Unsigned-integer packer with type deduction based on `W` (narrowest adequate type)
- `bp3k::bitpacker<typename T, std::size_t W, std::size_t N>`
  - Primary implementation (no type deduction)
  - `T` can be specialized with enumeration types

### `T` as Signed Type

Let's say we need an array of 6-bit signed integers with a size of 32, which is implemented explicitly by the following template specialization:
```cpp
bp3k::bitpacker<std::int8_t, 6, 32>;
```
The equivalent type can also be specialized with signed-type deduction via `bp3k::ibitpacker<W, N>`:
```cpp
bp3k::ibitpacker<6, 32>;
```
Here's a demonstration of the type's main functionality:
```cpp
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include "bp3k.h" // <bp3k.h> if installed

// Using ibitpacker for value-type deduction (might as well)
using ipack6x32 = bp3k::ibitpacker<6, 32>;

// Fetching the deduced value type
using T = typename ipack6x32::value_type;

inline void print_elements(const ipack6x32& bp) noexcept
{
  std::printf("%hhd", bp[0]);

  for (std::size_t i = 1; i < 32; ++i)
    std::printf(" %hhd", bp[i]);
  
  std::putc('\n', stdout);
}

int main(void)
{
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

  bp2 = bp1; // both filled with value_min
  bp2.back() = ipack6x32::value_min + 1; // make bp2 greater

  std::cout << "bp1 == bp2: " << std::boolalpha << (bp1 == bp2) << std::endl;
  std::cout << "bp1 < bp2: " << std::boolalpha << (bp1 < bp2) << std::endl;
  std::cout << "bp1 <= bp2: " << std::boolalpha << (bp1 <= bp2) << std::endl;
  std::cout << "bp1 > bp2: " << std::boolalpha << (bp1 > bp2) << std::endl;
  std::cout << "bp1 >= bp2: " << std::boolalpha << (bp1 >= bp2) << std::endl;

  return 0;
}
```

### `T` as Unsigned Type

Let's say we need an array of 4-bit unsigned integers with a size of 16, which is implemented explicitly by the following template specialization:
```cpp
bp3k::bitpacker<std::uint8_t, 4, 16>;
```
The equivalent type can also be specialized with signed-type deduction via `bp3k::ubitpacker<W, N>`:
```cpp
bp3k::ubitpacker<4, 16>;
```
Here's a demonstration of the type's main functionality:
```cpp
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include "bp3k.h" // <bp3k.h> if installed

// Using ibitpacker for value-type deduction (might as well)
using upack4x16 = bp3k::ubitpacker<4, 16>;

// Fetching the deduced value type
using T = typename upack4x16::value_type;

inline void print_elements(const upack4x16& bp) noexcept
{
  std::printf("%hhu", bp[0]);

  for (std::size_t i = 1; i < 16; ++i)
    std::printf(" %hhu", bp[i]);
  
  std::putc('\n', stdout);
}

int main(void)
{
  // Default constructor

  upack4x16 bp1;

  std::printf("bp1: ");
  print_elements(bp1);
  
  // Fill constructor (initializes all 16 elements to 13)

  upack4x16 bp2(13);

  std::printf("bp2: ");
  print_elements(bp2);

  // Value range (static constants)
  std::printf("value_min: %hhu\n", (T)upack4x16::value_min);
  std::printf("value_max: %hhu\n", (T)upack4x16::value_max);

  // Data access

  bp1[7] = upack4x16::value_max - 1;
  std::printf("bp1[7]: %hhd\n", (T)bp1[7]);

  bp2.at(7) = upack4x16::value_max - 2;
  std::printf("bp2.at(7): %hhd\n", (T)bp2.at(7));

  bp1.front() = upack4x16::value_max - 3;
  std::printf("bp1.front(): %hhd\n", (T)bp1.front());

  bp2.back() = upack4x16::value_max - 4;
  std::printf("bp2.back(): %hhd\n", (T)bp2.back());

  // Operations

  bp2.fill(upack4x16::value_min);
  bp1.swap(bp2);

  // Lexicographic comparison

  bp2 = bp1; // both filled with value_min
  bp2.back() = upack4x16::value_min + 1; // make bp2 greater

  std::cout << "bp1 == bp2: " << std::boolalpha << (bp1 == bp2) << std::endl;
  std::cout << "bp1 < bp2: " << std::boolalpha << (bp1 < bp2) << std::endl;
  std::cout << "bp1 <= bp2: " << std::boolalpha << (bp1 <= bp2) << std::endl;
  std::cout << "bp1 > bp2: " << std::boolalpha << (bp1 > bp2) << std::endl;
  std::cout << "bp1 >= bp2: " << std::boolalpha << (bp1 >= bp2) << std::endl;

  return 0;
}
```

### `T` as Enumeration Type

The following demonstrates usage of `bp3k::bitpacker<T, W, N>` with an enumeration type. 

```cpp
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include "bp3k.h" // <bp3k.h> if installed

enum class ChessPiece : std::int8_t {
  BlackPawn = -6,
  BlackKnight = -5,
  BlackBishop = -4,
  BlackRook = -3,
  BlackQueen = -2,
  BlackKing = -1,
  None = 0,
  WhiteKing = 1,
  WhiteQueen = 2,
  WhiteRook = 3,
  WhiteBishop = 4,
  WhiteKnight = 5,
  WhitePawn = 6
};

inline const char *chesspiece_to_str(ChessPiece p)
{
  static const char *lookup_table[13] = {
    "♟", "♞", "♝", "♜", "♛", "♚",
    " ",
    "♔", "♕", "♖", "♗", "♘", "♙"
  };

  auto i = (int)p + 6;

  if (i < 0 || i > 12)
    throw std::invalid_argument("undefined `ChessPiece' enumeration member");
  
  return lookup_table[i];
}

class ChessBoard final {
  bp3k::bitpacker<ChessPiece, 4, 64> matrix_{};

public:
  inline constexpr ChessBoard() noexcept;
  inline ChessPiece operator[](const std::string& pos) const;
  inline void move(const std::string& to, const std::string& from);
  inline void print(FILE *stream, bool black_view = false) const;
};

inline std::size_t pos_to_index(const std::string& pos)
{
  if (pos.size() != 2)
    throw std::invalid_argument("pos must match `^[A-Ha-h][1-8]$'");
  
  int non_upper = (int)(pos[0] > 'Z') * 32;
  int column = (int)pos[0] - non_upper - 'A';
  int row = 8 - (pos[1] - '0');

  if (column < 0 || column > 7 || row < 0 || row > 7)
    throw std::invalid_argument("pos must match `^[A-Ha-h][1-8]$'");
  
  return (std::size_t)(row * 8 + column);
}

constexpr ChessBoard::ChessBoard() noexcept
{
  this->matrix_[0] = ChessPiece::BlackRook;
  this->matrix_[1] = ChessPiece::BlackKnight;
  this->matrix_[2] = ChessPiece::BlackBishop;
  this->matrix_[3] = ChessPiece::BlackQueen;
  this->matrix_[4] = ChessPiece::BlackKing;
  this->matrix_[5] = ChessPiece::BlackBishop;
  this->matrix_[6] = ChessPiece::BlackKnight;
  this->matrix_[7] = ChessPiece::BlackRook;

  for (std::size_t i = 8; i < 16; ++i)
    this->matrix_[i] = ChessPiece::BlackPawn;

  for (std::size_t i = 48; i < 56; ++i)
    this->matrix_[i] = ChessPiece::WhitePawn;
  
  this->matrix_[56] = ChessPiece::WhiteRook;
  this->matrix_[57] = ChessPiece::WhiteKnight;
  this->matrix_[58] = ChessPiece::WhiteBishop;
  this->matrix_[59] = ChessPiece::WhiteQueen;
  this->matrix_[60] = ChessPiece::WhiteKing;
  this->matrix_[61] = ChessPiece::WhiteBishop;
  this->matrix_[62] = ChessPiece::WhiteKnight;
  this->matrix_[63] = ChessPiece::WhiteRook;
}

ChessPiece ChessBoard::operator[](const std::string& pos) const
{
  auto i = pos_to_index(pos);
  return this->matrix_[i];
}

void ChessBoard::move(const std::string& to, const std::string& from)
{
  auto to_i = pos_to_index(to);
  auto from_i = pos_to_index(from);

  if (this->matrix_[from_i] == ChessPiece::None)
    throw std::invalid_argument("Cannot move piece from vacant position");
  
  this->matrix_[to_i] = this->matrix_[from_i];
  this->matrix_[from_i] = ChessPiece::None;
}

void ChessBoard::print(FILE *stream, bool black_view) const
{
  if (black_view) {
    for (auto r = 7; r >= 0; --r) {
      for (auto c = 7; c >= 0; --c) {
        auto i = r * 8 + c;
        std::fprintf(stream, "%s", chesspiece_to_str(this->matrix_[i]));
      }

      std::fputc('\n', stream);
    }
  }
  else {
    for (auto r = 0; r < 8; ++r) {
      for (auto c = 0; c < 8; ++c) {
        auto i = r * 8 + c;
        std::fprintf(stream, "%s", chesspiece_to_str(this->matrix_[i]));
      }

      std::fputc('\n', stream);
    }
  }
}

int main(void)
{
  ChessBoard board;

  board.move("e4", "e2");
  board.move("e5", "e7");

  std::cout << "[White Player Perspective]" << std::endl << std::endl;
  board.print(stdout);

  std::cout << std::endl << "[Black Player Perspective]" << std::endl << std::endl;
  board.print(stdout, true);
}
```

(See documentation for full capabilities.)
