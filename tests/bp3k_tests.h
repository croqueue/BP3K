#ifndef _BITPACKER3000_TESTS_H_
#define _BITPACKER3000_TESTS_H_

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <type_traits>

#include "bp3k.h"

namespace bp3k::tests {

// Convenience typedefs

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// Dummy enum types (members enumerated for up to 4 bits)

enum class i8enum : i8 {
  MinusEight = -8,
  MinusSeven = -7,
  MinusSix = -6,
  MinusFive = -5,
  MinusFour = -4,
  MinusThree = -3,
  MinusTwo = -2,
  MinusOne = -1,
  Zero = 0,
  One = 1,
  Two = 2,
  Three = 3,
  Four = 4,
  Five = 5,
  Six = 6,
  Seven = 7
};

enum class u8enum : u8 {
  Zero = 0,
  One = 1,
  Two = 2,
  Three = 3,
  Four = 4,
  Five = 5,
  Six = 6,
  Seven = 7,
  Eight = 8,
  Nine = 9,
  Ten = 10,
  Eleven = 11,
  Twelve = 12,
  Thirteen = 13,
  Fourteen = 14,
  Fifteen = 15
};

template <typename T, std::size_t W, std::size_t N>
class BitPackerTests : public ::testing::Test {
 protected:
  using underlying_type =
      typename bp3k::impl::impl_type_map<T, std::is_enum<T>::value>::type;
  using bitpacker_type = bp3k::bitpacker<T, W, N>;

  static constexpr underlying_type value_min =
      (underlying_type)bitpacker_type::value_min;
  static constexpr underlying_type value_max =
      (underlying_type)bitpacker_type::value_max;

  bitpacker_type lhs_, rhs_;
  std::array<T, N> lhs_vals_, rhs_vals_;
  std::mt19937 rng_;
  std::uniform_int_distribution<underlying_type> rng_dist_;

  inline constexpr T random_next() noexcept {
    return static_cast<T>(this->rng_dist_(this->rng_));
  }

  inline constexpr void populate_unique_arrays() noexcept {
    T l{}, r{};

    for (std::size_t i = 0; i < N; ++i) {
      l = this->random_next();

      // Ensure l/r are different values (for reliable testing of swap)
      while ((r = this->random_next()) == l)
        ;

      this->lhs_[i] = l;
      this->rhs_[i] = r;
      this->lhs_vals_[i] = l;
      this->rhs_vals_[i] = r;
    }
  }

 public:
  BitPackerTests()
      : lhs_{},
        rhs_{},
        lhs_vals_{},
        rhs_vals_{},
        rng_{1337},
        rng_dist_(value_min, value_max) {}
};

template <std::size_t W, std::size_t N>
class IBitPackerTests : public ::testing::Test {};

template <std::size_t W, std::size_t N>
class UBitPackerTests : public ::testing::Test {};

}  // namespace bp3k::tests

#define _BITPACKER_TEST_NAME_(t, w, n) BitPacker_##T##t##_W##w##_N##n##_Tests

#define _IBITPACKER_TEST_NAME_(w, n) IBitPacker_##W##w##_N##n##_Tests

#define _UBITPACKER_TEST_NAME_(w, n) UBitPacker_##W##w##_N##n##_Tests

#define BITPACKER_TEST_SUITE(T, W, N)                                          \
                                                                               \
  using _BITPACKER_TEST_NAME_(T, W, N) = bp3k::tests::BitPackerTests<T, W, N>; \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), DefaultConstructor) {                 \
    bitpacker_type bp;                                                         \
                                                                               \
    for (std::size_t i = 0; i < N; ++i) {                                      \
      ASSERT_EQ(bp[i], (T)0);                                                  \
      ASSERT_EQ(bp.at(i), (T)0);                                               \
    }                                                                          \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), FillConstructor) {                    \
    T fill_value = this->random_next();                                        \
    bitpacker_type bp(fill_value);                                             \
                                                                               \
    for (std::size_t i = 0; i < N; ++i) {                                      \
      T bp_i = bp[i];                                                          \
      ASSERT_EQ(bp_i, fill_value);                                             \
    }                                                                          \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), ItemAccess) {                         \
    T a = this->random_next(), b{};                                            \
                                                                               \
    while ((b = this->random_next()) == a)                                     \
      ;                                                                        \
                                                                               \
    for (std::size_t i = 0; i < N; ++i) {                                      \
      /* Test non-const at(), const operator[] */                              \
      this->lhs_.at(i) = a;                                                    \
      ASSERT_TRUE(this->lhs_[i] == a);                                         \
                                                                               \
      /* Test non-const operator[], const at() */                              \
      this->lhs_[i] = b;                                                       \
      ASSERT_TRUE(this->lhs_.at(i) == b);                                      \
    }                                                                          \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), Front) {                              \
    T a = this->random_next(), b{};                                            \
                                                                               \
    while ((b = this->random_next()) == a)                                     \
      ;                                                                        \
                                                                               \
    /* Test non-const overload (reference) */                                  \
    this->lhs_.front() = a;                                                    \
    ASSERT_EQ(this->lhs_[0], a);                                               \
                                                                               \
    /* Test const overload (extracted value) */                                \
    this->lhs_[0] = b;                                                         \
    ASSERT_EQ(this->lhs_.front(), b);                                          \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), Back) {                               \
    T a = this->random_next(), b{};                                            \
                                                                               \
    while ((b = this->random_next()) == a)                                     \
      ;                                                                        \
                                                                               \
    /* Test non-const (reference) */                                           \
    this->lhs_.back() = a;                                                     \
    ASSERT_EQ(this->lhs_[N - 1], a);                                           \
                                                                               \
    /* Test const overload (extracted value) */                                \
    this->lhs_[N - 1] = b;                                                     \
    ASSERT_EQ(this->lhs_.back(), b);                                           \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), Data) {                               \
    std::uintmax_t *bp_address = (std::uintmax_t *)&this->lhs_;                \
    /* Non-const overload */                                                   \
    std::uintmax_t *data = this->lhs_.data();                                  \
    /* Const overload */                                                       \
    const std::uintmax_t *cdata = this->lhs_.data();                           \
                                                                               \
    ASSERT_EQ(data, bp_address);                                               \
    ASSERT_EQ(cdata, bp_address);                                              \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), Empty) {                              \
    ASSERT_EQ(this->lhs_.empty(), N == 0);                                     \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), Size) {                               \
    ASSERT_EQ(this->lhs_.size(), N);                                           \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), MaxSize) {                            \
    constexpr std::size_t bp_size = sizeof(bitpacker_type);                    \
    constexpr std::size_t word_size = sizeof(std::uintmax_t);                  \
    constexpr std::size_t word_count = bp_size / word_size;                    \
    constexpr std::size_t word_width = word_size << 3;                         \
    constexpr std::size_t per_word = word_width / W;                           \
                                                                               \
    ASSERT_EQ(this->lhs_.max_size(), word_count *per_word);                    \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), Swap) {                               \
    this->populate_unique_arrays();                                            \
                                                                               \
    /* Swap and cross-compare with arrays */                                   \
    this->lhs_.swap(this->rhs_);                                               \
                                                                               \
    for (std::size_t i = 0; i < N; ++i) {                                      \
      ASSERT_EQ(this->lhs_[i], this->rhs_vals_[i]);                            \
      ASSERT_EQ(this->rhs_[i], this->lhs_vals_[i]);                            \
    }                                                                          \
                                                                               \
    /* Swap back and compare again */                                          \
    this->rhs_.swap(this->lhs_);                                               \
                                                                               \
    for (std::size_t i = 0; i < N; ++i) {                                      \
      ASSERT_EQ(this->lhs_[i], this->lhs_vals_[i]);                            \
      ASSERT_EQ(this->rhs_[i], this->rhs_vals_[i]);                            \
    }                                                                          \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), Fill) {                               \
    T value = this->random_next();                                             \
                                                                               \
    this->lhs_.fill(value);                                                    \
                                                                               \
    for (std::size_t i = 0; i < N; ++i) {                                      \
      ASSERT_EQ(this->lhs_[i], value);                                         \
      ASSERT_EQ(this->lhs_.at(i), value);                                      \
    }                                                                          \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), CompareEQ) {                          \
    T value = this->random_next();                                             \
                                                                               \
    this->lhs_.fill(value);                                                    \
    this->rhs_.fill(value);                                                    \
                                                                               \
    ASSERT_TRUE(this->lhs_ == this->rhs_);                                     \
                                                                               \
    this->lhs_[N - 1] = (T) ~((underlying_type)value);                         \
                                                                               \
    ASSERT_FALSE(this->lhs_ == this->rhs_);                                    \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), CompareNE) {                          \
    T value = this->random_next();                                             \
                                                                               \
    this->lhs_.fill(value);                                                    \
    this->rhs_.fill(value);                                                    \
                                                                               \
    ASSERT_FALSE(this->lhs_ != this->rhs_);                                    \
                                                                               \
    this->lhs_[N - 1] = (T) ~((underlying_type)value);                         \
                                                                               \
    ASSERT_TRUE(this->lhs_ != this->rhs_);                                     \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), CompareLT) {                          \
    this->lhs_.fill((T)0);                                                     \
    this->rhs_.fill((T)0);                                                     \
                                                                               \
    this->rhs_[1] = (T)1;                                                      \
                                                                               \
    ASSERT_TRUE(this->lhs_ < this->rhs_);                                      \
    ASSERT_FALSE(this->rhs_ < this->lhs_);                                     \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), CompareLE) {                          \
    this->lhs_.fill((T)0);                                                     \
    this->rhs_.fill((T)0);                                                     \
                                                                               \
    ASSERT_TRUE(this->lhs_ <= this->rhs_);                                     \
                                                                               \
    this->rhs_[1] = (T)1;                                                      \
                                                                               \
    ASSERT_TRUE(this->lhs_ <= this->rhs_);                                     \
    ASSERT_FALSE(this->rhs_ <= this->lhs_);                                    \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), CompareGT) {                          \
    this->lhs_.fill((T)0);                                                     \
    this->rhs_.fill((T)0);                                                     \
                                                                               \
    this->lhs_[1] = (T)1;                                                      \
                                                                               \
    ASSERT_TRUE(this->lhs_ > this->rhs_);                                      \
    ASSERT_FALSE(this->rhs_ > this->lhs_);                                     \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), CompareGE) {                          \
    this->lhs_.fill((T)0);                                                     \
    this->rhs_.fill((T)0);                                                     \
                                                                               \
    ASSERT_TRUE(this->lhs_ >= this->rhs_);                                     \
                                                                               \
    this->lhs_[1] = (T)1;                                                      \
                                                                               \
    ASSERT_TRUE(this->lhs_ >= this->rhs_);                                     \
    ASSERT_FALSE(this->rhs_ >= this->lhs_);                                    \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), ProxyAssignmentAccessOp) {            \
    this->populate_unique_arrays();                                            \
                                                                               \
    for (std::size_t i = 0; i < N; ++i) {                                      \
      this->lhs_[i] = this->rhs_[i];                                           \
      ASSERT_EQ(lhs_[i], this->rhs_vals_[i]);                                  \
    }                                                                          \
  }                                                                            \
                                                                               \
  TEST_F(_BITPACKER_TEST_NAME_(T, W, N), ProxyAssignmentAtMethod) {            \
    this->populate_unique_arrays();                                            \
                                                                               \
    for (std::size_t i = 0; i < N; ++i) {                                      \
      this->lhs_.at(i) = this->rhs_.at(i);                                     \
      ASSERT_EQ(lhs_.at(i), this->rhs_vals_[i]);                               \
    }                                                                          \
  }

#define IBITPACKER_TEST_SUITE(W, N)                                         \
                                                                            \
  using _IBITPACKER_TEST_NAME_(W, N) = bp3k::tests::IBitPackerTests<W, N>;  \
                                                                            \
  TEST_F(_IBITPACKER_TEST_NAME_(W, N), ValueTypeCorrect) {                  \
    using T = typename bp3k::ibitpacker<W, N>::value_type;                  \
                                                                            \
    if constexpr (W <= 8) {                                                 \
      constexpr bool match = std::is_same<i8, T>::value;                    \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 16) {                                         \
      constexpr bool match = std::is_same<i16, T>::value;                   \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 32) {                                         \
      constexpr bool match = std::is_same<i32, T>::value;                   \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 64) {                                         \
      constexpr bool match = std::is_same<i64, T>::value;                   \
      ASSERT_TRUE(match);                                                   \
    } else {                                                                \
      ASSERT_TRUE(false);                                                   \
    }                                                                       \
  }                                                                         \
                                                                            \
  TEST_F(_IBITPACKER_TEST_NAME_(W, N), HasCorrectBitPackerType) {           \
    using ibitpacker_type = bp3k::ibitpacker<W, N>;                         \
    if constexpr (W <= 8) {                                                 \
      constexpr bool match =                                                \
          std::is_same<ibitpacker_type, bp3k::bitpacker<i8, W, N>>::value;  \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 16) {                                         \
      constexpr bool match =                                                \
          std::is_same<ibitpacker_type, bp3k::bitpacker<i16, W, N>>::value; \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 32) {                                         \
      constexpr bool match =                                                \
          std::is_same<ibitpacker_type, bp3k::bitpacker<i32, W, N>>::value; \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 64) {                                         \
      constexpr bool match =                                                \
          std::is_same<ibitpacker_type, bp3k::bitpacker<i64, W, N>>::value; \
      ASSERT_TRUE(match);                                                   \
    } else {                                                                \
      ASSERT_TRUE(false);                                                   \
    }                                                                       \
  }

#define UBITPACKER_TEST_SUITE(W, N)                                         \
                                                                            \
  using _UBITPACKER_TEST_NAME_(W, N) = bp3k::tests::UBitPackerTests<W, N>;  \
                                                                            \
  TEST_F(_UBITPACKER_TEST_NAME_(W, N), ValueTypeCorrect) {                  \
    using T = typename bp3k::ubitpacker<W, N>::value_type;                  \
    if constexpr (W <= 8) {                                                 \
      constexpr bool match = std::is_same<u8, T>::value;                    \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 16) {                                         \
      constexpr bool match = std::is_same<u16, T>::value;                   \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 32) {                                         \
      constexpr bool match = std::is_same<u32, T>::value;                   \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 64) {                                         \
      constexpr bool match = std::is_same<u64, T>::value;                   \
      ASSERT_TRUE(match);                                                   \
    } else {                                                                \
      ASSERT_TRUE(false);                                                   \
    }                                                                       \
  }                                                                         \
                                                                            \
  TEST_F(_UBITPACKER_TEST_NAME_(W, N), HasCorrectBitPackerType) {           \
    using ubitpacker_type = bp3k::ubitpacker<W, N>;                         \
    if constexpr (W <= 8) {                                                 \
      constexpr bool match =                                                \
          std::is_same<ubitpacker_type, bp3k::bitpacker<u8, W, N>>::value;  \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 16) {                                         \
      constexpr bool match =                                                \
          std::is_same<ubitpacker_type, bp3k::bitpacker<u16, W, N>>::value; \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 32) {                                         \
      constexpr bool match =                                                \
          std::is_same<ubitpacker_type, bp3k::bitpacker<u32, W, N>>::value; \
      ASSERT_TRUE(match);                                                   \
    } else if constexpr (W <= 64) {                                         \
      constexpr bool match =                                                \
          std::is_same<ubitpacker_type, bp3k::bitpacker<u64, W, N>>::value; \
      ASSERT_TRUE(match);                                                   \
    } else {                                                                \
      ASSERT_TRUE(false);                                                   \
    }                                                                       \
  }

#endif  // !_BITPACKER3000_TESTS_H_
