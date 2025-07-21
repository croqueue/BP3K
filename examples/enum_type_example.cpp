#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

#include "bp3k.h"  // <bp3k.h> if installed

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

inline const char* chesspiece_to_str(ChessPiece p) {
  static const char* lookup_table[13] = {"♟", "♞", "♝", "♜", "♛", "♚", " ",
                                         "♔", "♕", "♖", "♗", "♘", "♙"};

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
  inline void print(FILE* stream, bool black_view = false) const;
};

inline std::size_t pos_to_index(const std::string& pos) {
  if (pos.size() != 2)
    throw std::invalid_argument("pos must match `^[A-Ha-h][1-8]$'");

  int non_upper = (int)(pos[0] > 'Z') * 32;
  int column = (int)pos[0] - non_upper - 'A';
  int row = 8 - (pos[1] - '0');

  if (column < 0 || column > 7 || row < 0 || row > 7)
    throw std::invalid_argument("pos must match `^[A-Ha-h][1-8]$'");

  return (std::size_t)(row * 8 + column);
}

constexpr ChessBoard::ChessBoard() noexcept {
  this->matrix_[0] = ChessPiece::BlackRook;
  this->matrix_[1] = ChessPiece::BlackKnight;
  this->matrix_[2] = ChessPiece::BlackBishop;
  this->matrix_[3] = ChessPiece::BlackQueen;
  this->matrix_[4] = ChessPiece::BlackKing;
  this->matrix_[5] = ChessPiece::BlackBishop;
  this->matrix_[6] = ChessPiece::BlackKnight;
  this->matrix_[7] = ChessPiece::BlackRook;

  for (std::size_t i = 8; i < 16; ++i) this->matrix_[i] = ChessPiece::BlackPawn;

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

ChessPiece ChessBoard::operator[](const std::string& pos) const {
  auto i = pos_to_index(pos);
  return this->matrix_[i];
}

void ChessBoard::move(const std::string& to, const std::string& from) {
  auto to_i = pos_to_index(to);
  auto from_i = pos_to_index(from);

  if (this->matrix_[from_i] == ChessPiece::None)
    throw std::invalid_argument("Cannot move piece from vacant position");

  this->matrix_[to_i] = this->matrix_[from_i];
  this->matrix_[from_i] = ChessPiece::None;
}

void ChessBoard::print(FILE* stream, bool black_view) const {
  if (black_view) {
    for (auto r = 7; r >= 0; --r) {
      for (auto c = 7; c >= 0; --c) {
        auto i = r * 8 + c;
        std::fprintf(stream, "%s", chesspiece_to_str(this->matrix_[i]));
      }

      std::fputc('\n', stream);
    }
  } else {
    for (auto r = 0; r < 8; ++r) {
      for (auto c = 0; c < 8; ++c) {
        auto i = r * 8 + c;
        std::fprintf(stream, "%s", chesspiece_to_str(this->matrix_[i]));
      }

      std::fputc('\n', stream);
    }
  }
}

int main(void) {
  ChessBoard board;

  board.move("e4", "e2");
  board.move("e5", "e7");

  std::cout << "[White Player Perspective]" << std::endl << std::endl;
  board.print(stdout);

  std::cout << std::endl
            << "[Black Player Perspective]" << std::endl
            << std::endl;
  board.print(stdout, true);
}
