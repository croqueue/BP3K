#ifndef _BITPACKER3000_H_
#define _BITPACKER3000_H_

#include <cstddef>
#include <cstdint>
#include <type_traits>

/// @internal
/// @brief Implementation details
namespace bp3k::impl {

/// @brief Checks if a compile-time constant is a power of 2
/// @tparam N Constant value
template <std::size_t N>
constexpr bool is_power_of_2 = (N != 0) && !(N & (N - 1));

/// @brief Computes compile-time constant's left shift to MSB
/// @tparam N Constant value
/// @return Result of computation
template <std::size_t N>
inline constexpr std::size_t msb_log2() noexcept {
  constexpr std::size_t max_offset = (sizeof(std::size_t) << 3) - 1;

  static_assert(N != 0, "constexpr is invalid for N=0");

  std::size_t left_offset = 0;
  std::size_t mask = (std::size_t)1 << max_offset;

  for (; left_offset <= max_offset; ++left_offset) {
    if (N & mask) break;

    mask >>= 1;
  }

  return max_offset - left_offset;
}

/// @brief Computes compile-time contant's smallest power of 2 >= N
/// @tparam N Constant value
/// @return Result of computation
template <std::size_t N>
inline constexpr std::size_t ceil_power_of_2() noexcept {
  if constexpr (N == 0)
    return 0;
  else if constexpr (N <= 8)
    return 8;
  else if constexpr (is_power_of_2<N>)
    return N;

  return 1 << (msb_log2<N>() + 1);
}

/// @brief Signed-type mapper
/// @tparam W Bit width
template <std::size_t W>
struct signed_type_map final {
  /// @brief Signed type that matches width
  using type = void;
};

template <>
struct signed_type_map<8> final {
  using type = std::int8_t;
};

template <>
struct signed_type_map<16> final {
  using type = std::int16_t;
};

template <>
struct signed_type_map<32> final {
  using type = std::int32_t;
};

template <>
struct signed_type_map<64> final {
  using type = std::int64_t;
};

/// @brief Signed-type dispatcher for arbitrary width
/// @tparam W Bit width
template <std::size_t W>
using fit_signed = typename signed_type_map<ceil_power_of_2<W>()>::type;

/// @brief Unsigned-type mapper
/// @tparam W Bit width
template <std::size_t W>
struct unsigned_type_map final {
  /// @brief Unsigned type that matches width
  using type = void;
};

template <>
struct unsigned_type_map<8> final {
  using type = std::uint8_t;
};

template <>
struct unsigned_type_map<16> final {
  using type = std::uint16_t;
};

template <>
struct unsigned_type_map<32> final {
  using type = std::uint32_t;
};

template <>
struct unsigned_type_map<64> final {
  using type = std::uint64_t;
};

/// @brief Unsigned-type dispatcher for arbitrary width
/// @tparam W Bit width
template <std::size_t W>
using fit_unsigned = typename unsigned_type_map<ceil_power_of_2<W>()>::type;

/// @brief Underlying-type mapper (to abstract integral types vs enums)
/// @tparam T Integral or enumeration type
/// @tparam IsEnum
template <typename T, bool IsEnum>
struct impl_type_map final {
  /// @brief T or its underlying type
  using type = void;
};

template <typename T>
struct impl_type_map<T, false> final {
  using type = T;
};

template <typename T>
struct impl_type_map<T, true> final {
  using type = typename std::underlying_type<T>::type;
};

/// @brief Underlying-type dispatcher (to abstract integral types vs enums)
/// @tparam T Integral or enumeration type
template <typename T>
using impl_type = typename impl_type_map<T, std::is_enum<T>::value>::type;

/// @brief Dispatches bit-packing operations based on type configuration
/// @tparam T I/O value type (signed/unsigned integral or enumeration type)
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
template <typename T, std::size_t W, std::size_t N>
class type_dispatcher final {
  static constexpr bool t_is_enum = std::is_enum<T>::value;
  static constexpr std::size_t t_width = sizeof(T) << 3;
  using integral_type = impl_type<T>;

  static_assert(std::is_integral<T>::value || t_is_enum,
                "T must be an integral or enumeration type");
  static_assert(W <= t_width, "T cannot hold W bits");

 public:
  using unsigned_type = typename std::make_unsigned<integral_type>::type;

  static constexpr bool t_is_signed = std::is_signed<integral_type>::value;
  static constexpr std::size_t w_log2 = msb_log2<W>();
  static constexpr std::size_t word_width = sizeof(std::uintmax_t) << 3;
  static constexpr std::size_t word_width_log2 = msb_log2<word_width>();
  static constexpr std::size_t per_word = word_width / W;
  static constexpr std::size_t per_word_log2 = word_width_log2 - w_log2;
  static constexpr std::size_t extra_items = N % per_word;
  static constexpr std::size_t last_word_items =
      extra_items > 0 ? extra_items : per_word;
  static constexpr std::size_t word_count =
      (N / per_word) + (std::size_t)(per_word != last_word_items);
  static constexpr std::size_t max_size = word_count * per_word;
  static constexpr unsigned_type value_mask = (unsigned_type)((1 << W) - 1);
  static constexpr unsigned_type sign_extend_bits = (unsigned_type)~value_mask;
  static constexpr unsigned_type sign_bit_mask = (unsigned_type)(1 << (W - 1));
  static constexpr bool w_is_power_of_2 = is_power_of_2<W>;
  static constexpr std::size_t front_offset = word_width - W;
  static constexpr std::size_t back_offset =
      word_width - (W * (last_word_items));

  /// @brief Computes minimum value for width W
  /// @return Result of computation
  static inline constexpr T value_min() noexcept {
    if constexpr (!t_is_signed) return (T)0;

    return (T)(-(integral_type)sign_bit_mask);
  }

  /// @brief Computes maximum value for width W
  /// @return Result of computation
  static inline constexpr T value_max() noexcept {
    if constexpr (!t_is_signed) return (T)value_mask;

    return (T)(value_mask >> 1);
  }

  /// @brief Computes word index
  /// @param item_pos Index of item in the bitpacker
  /// @return Index of word
  static inline constexpr std::size_t word_index(
      std::size_t item_pos) noexcept {
    if constexpr (w_is_power_of_2)
      return item_pos >> per_word_log2;
    else
      return item_pos / per_word;
  }

  /// @brief Computes shift offset within word
  /// @param item_pos Index of item in the bitpacker
  /// @return Item shift offset
  static inline constexpr std::size_t item_offset(
      std::size_t item_pos) noexcept {
    if constexpr (w_is_power_of_2) {
      auto pos_in_word = item_pos & (per_word - 1);
      return front_offset - (pos_in_word * W);
    } else {
      auto pos_in_word = item_pos % per_word;
      return front_offset - (pos_in_word * W);
    }
  }

  /// @brief Determine if packed bits represent a negative value
  /// @param w_bits Raw packed bits
  /// @return True if value is negative
  static inline constexpr unsigned_type is_negative(
      unsigned_type w_bits) noexcept {
    return (unsigned_type)((w_bits & sign_bit_mask) != 0);
  }

  /// @brief Performs sign extension on packed bits
  /// @param w_bits Raw packed bits
  /// @return Sign-extended form of the raw bits
  static inline constexpr unsigned_type extend_sign(
      unsigned_type w_bits) noexcept {
    unsigned_type mask = sign_extend_bits * is_negative(w_bits);
    return w_bits | mask;
  }

  /// @brief Packs a value into a word
  /// @param word_ptr Pointer to word
  /// @param offset Item offset within word
  /// @param value Value to embed
  static inline constexpr void embed_value(std::uintmax_t* word_ptr,
                                           std::size_t offset,
                                           unsigned_type value) noexcept {
    auto clear_mask = ~((std::uintmax_t)value_mask << offset);
    auto w_bits = (std::uintmax_t)(value & value_mask);

    *word_ptr &= clear_mask;
    *word_ptr |= w_bits << offset;
  }

  /// @brief Unpacks a value from a word
  /// @param word_ptr Pointer to word
  /// @param offset Item offset within word
  /// @return Unpacked value
  static inline constexpr unsigned_type extract_value(
      std::uintmax_t* word_ptr, std::size_t offset) noexcept {
    auto w_bits = static_cast<unsigned_type>(*word_ptr >> offset) & value_mask;

    if constexpr (!t_is_signed) return w_bits;

    return extend_sign(w_bits);
  }

  /// @brief Fills a word with a repeated packed value
  /// @tparam PackCount Number of values to embed
  /// @param mask0 Left-most value mask
  /// @return The packed word
  template <std::size_t PackCount>
  static inline constexpr std::uintmax_t fill_word(
      std::uintmax_t mask0) noexcept {
    std::uintmax_t word{};

    for (std::size_t i = 0; i < PackCount; ++i) {
      word |= mask0;
      mask0 >>= W;
    }

    return word;
  }

  /// @brief Fills word buffer with a repeated packed value
  /// @param word_ptr Pointer to first word
  /// @param value Fill value
  static inline constexpr void fill_buffer(std::uintmax_t* word_ptr,
                                           unsigned_type value) noexcept {
    std::uintmax_t mask0 = static_cast<std::uintmax_t>(value & value_mask)
                           << front_offset;
    std::size_t i{};

    for (i = 0; i < word_count - 1; ++i)
      word_ptr[i] = fill_word<per_word>(mask0);

    word_ptr[i] = fill_word<last_word_items>(mask0);
  }
};

}  // namespace bp3k::impl

/// @namespace bp3k
/// @brief Root namespace for the BP3K library
namespace bp3k {

/// @brief Bit-packing array template
/// @tparam T I/O value type (signed/unsigned integral or enumeration type)
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
template <typename T, std::size_t W, std::size_t N>
class bitpacker final {
  using type_dispatcher = impl::type_dispatcher<T, W, N>;

  std::uintmax_t data_[type_dispatcher::word_count]{};

 public:
  /// @brief
  friend class item_proxy;

  /// @brief Proxy object that provides reference to packed item
  class item_proxy final {
    /// @brief Pointer to word that holds the packed item
    std::uintmax_t* const word_ptr_;
    /// @brief Offset of packed item within word
    const std::size_t offset_;

    /// @brief Constructor for item referencing
    /// @param word_ptr Pointer to word
    /// @param offset Offset within word
    inline constexpr item_proxy(std::uintmax_t* word_ptr,
                                std::size_t offset) noexcept
        : word_ptr_(word_ptr), offset_(offset) {}

    /// @brief
    friend class bitpacker;

   public:
    /// @brief Copy constructor (default)
    /// @param
    item_proxy(const item_proxy&) = default;

    /// @brief Destructor (default)
    ~item_proxy() = default;

    /// @brief Assigns value to referenced item
    /// @param x Value
    /// @return Self reference
    inline constexpr item_proxy& operator=(T x) noexcept {
      using unsigned_type = typename type_dispatcher::unsigned_type;

      type_dispatcher::embed_value(this->word_ptr_, this->offset_,
                                   static_cast<unsigned_type>(x));
      return *this;
    }

    /// @brief Assigns value to referenced item from another referenced item
    /// @param rhs Reference to other item
    /// @return Self reference
    inline constexpr item_proxy& operator=(const item_proxy& rhs) noexcept {
      auto value = type_dispatcher::extract_value(rhs.word_ptr_, rhs.offset_);
      type_dispatcher::embed_value(this->word_ptr_, this->offset_, value);
      return *this;
    }

    /// @brief Extracts value from referenced item
    inline constexpr operator T() const noexcept {
      return (T)type_dispatcher::extract_value(this->word_ptr_, this->offset_);
    }
  };

  /// @brief T
  using value_type = T;

  /// @brief Storage type
  using word_type = std::uintmax_t;

  /// @brief bitpacker<T, W, N>::item_proxy
  using reference = item_proxy;

  /// @brief const bitpacker<T, W, N>::item_proxy
  using const_reference = const item_proxy;

  /// @brief Minimum value of T with width W
  static constexpr T value_min = type_dispatcher::value_min();

  /// @brief Maximum value of T with width W
  static constexpr T value_max = type_dispatcher::value_max();

  /// @brief Default constructor
  bitpacker() = default;

  /// @brief Fill constructor
  /// @param x Fill value
  inline constexpr bitpacker(T x) noexcept {
    using unsigned_type = typename type_dispatcher::unsigned_type;

    type_dispatcher::fill_buffer(this->data_, static_cast<unsigned_type>(x));
  }

  /// @brief Fetches reference to packed item
  /// @param pos Index of item
  /// @return Reference to packed item
  inline constexpr reference at(std::size_t pos) noexcept {
    auto word_index = type_dispatcher::word_index(pos);
    auto offset = type_dispatcher::item_offset(pos);
    return reference(&this->data_[word_index], offset);
  }

  /// @brief Fetches value of packed item
  /// @param pos Index of item
  /// @return Extracted value
  inline constexpr T at(std::size_t pos) const noexcept {
    auto word_index = type_dispatcher::word_index(pos);
    auto offset = type_dispatcher::item_offset(pos);
    auto w_bits =
        type_dispatcher::extract_value(&this->data_[word_index], offset);
    return static_cast<T>(w_bits);
  }

  /// @brief Fetches reference to packed item
  /// @param pos Index of item
  /// @return Reference to packed item
  inline constexpr reference operator[](std::size_t pos) noexcept {
    auto word_index = type_dispatcher::word_index(pos);
    auto offset = type_dispatcher::item_offset(pos);
    return reference(&this->data_[word_index], offset);
  }

  /// @brief Fetches value of packed item
  /// @param pos Index of item
  /// @return Extracted value
  inline constexpr T operator[](std::size_t pos) const noexcept {
    auto word_index = type_dispatcher::word_index(pos);
    auto offset = type_dispatcher::item_offset(pos);
    auto w_bits =
        type_dispatcher::extract_value(&this->data_[word_index], offset);
    return static_cast<T>(w_bits);
  }

  /// @brief Fetches reference to first item
  /// @return Reference to first item
  inline constexpr reference front() noexcept {
    return reference(&this->data_[0], type_dispatcher::front_offset);
  }

  /// @brief Fetches value of first item
  /// @return Value of first item
  inline constexpr T front() const noexcept {
    auto w_bits = type_dispatcher::extract_value(&this->data_[0],
                                                 type_dispatcher::front_offset);
    return static_cast<T>(w_bits);
  }

  /// @brief Fetches reference to last item
  /// @return Reference to last item
  inline constexpr reference back() noexcept {
    return reference(&this->data_[type_dispatcher::word_count - 1],
                     type_dispatcher::back_offset);
  }

  /// @brief Fetches value of last item
  /// @return Value of last item
  inline constexpr T back() const noexcept {
    auto w_bits = type_dispatcher::extract_value(
        &this->data_[type_dispatcher::word_count - 1],
        type_dispatcher::back_offset);
    return static_cast<T>(w_bits);
  }

  /// @brief Fetches address of private word buffer
  /// @return Pointer to private word buffer
  inline constexpr std::uintmax_t* data() noexcept { return this->data_; }

  /// @brief Fetches address of private word buffer
  /// @return Const pointer to private word buffer
  inline constexpr const std::uintmax_t* data() const noexcept {
    return this->data_;
  }

  /// @brief Checks if the container has no elements
  /// @return `true` if the container is empty, `false` otherwise
  inline constexpr bool empty() const noexcept { return N == 0; }

  /// @brief Returns the number of elements in the container
  /// @return N
  inline constexpr std::size_t size() const noexcept { return N; }

  /// @brief Returns the maximum number of elemts the container is able to hold
  /// @return Maximum number of elements
  inline constexpr std::size_t max_size() const noexcept {
    return type_dispatcher::max_size;
  }

  /// @brief Assigns `x` to all elements in the container
  /// @param x The value to assign
  inline constexpr void fill(T x) noexcept {
    using unsigned_type = typename type_dispatcher::unsigned_type;

    type_dispatcher::fill_buffer(this->data_, static_cast<unsigned_type>(x));
  }

  /// @brief Exchanges the contents of the container with those of `other`
  /// @param rhs
  inline constexpr void swap(bitpacker& other) noexcept {
    std::uintmax_t tmp{};

    for (std::size_t i = 0; i < type_dispatcher::word_count; ++i) {
      tmp = this->data_[i];
      this->data_[i] = other.data_[i];
      other.data_[i] = tmp;
    }
  }

  template <typename T_, std::size_t W_, std::size_t N_>
  friend constexpr bool operator==(const bitpacker<T_, W_, N_>& lhs,
                                   const bitpacker<T_, W_, N_>& rhs) noexcept;

  template <typename T_, std::size_t W_, std::size_t N_>
  friend constexpr bool operator!=(const bitpacker<T_, W_, N_>& lhs,
                                   const bitpacker<T_, W_, N_>& rhs) noexcept;

  template <typename T_, std::size_t W_, std::size_t N_>
  friend constexpr bool operator<(const bitpacker<T_, W_, N_>& lhs,
                                  const bitpacker<T_, W_, N_>& rhs) noexcept;

  template <typename T_, std::size_t W_, std::size_t N_>
  friend constexpr bool operator<=(const bitpacker<T_, W_, N_>& lhs,
                                   const bitpacker<T_, W_, N_>& rhs) noexcept;

  template <typename T_, std::size_t W_, std::size_t N_>
  friend constexpr bool operator>(const bitpacker<T_, W_, N_>& lhs,
                                  const bitpacker<T_, W_, N_>& rhs) noexcept;

  template <typename T_, std::size_t W_, std::size_t N_>
  friend constexpr bool operator>=(const bitpacker<T_, W_, N_>& lhs,
                                   const bitpacker<T_, W_, N_>& rhs) noexcept;
};

/// @brief Lexicographically compares two containers
/// @tparam T I/O value type (signed/unsigned integral or enumeration type)
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
/// @param lhs Left operand
/// @param rhs Right operand
/// @return `lhs == rhs`
template <typename T, std::size_t W, std::size_t N>
inline constexpr bool operator==(const bitpacker<T, W, N>& lhs,
                                 const bitpacker<T, W, N>& rhs) noexcept {
  using type_dispatcher = typename bitpacker<T, W, N>::type_dispatcher;

  for (std::size_t i = 0; i < type_dispatcher::word_count; ++i) {
    if (lhs.data_[i] != rhs.data_[i]) return false;
  }

  return true;
}

/// @brief Lexicographically compares two containers
/// @tparam T I/O value type (signed/unsigned integral or enumeration type)
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
/// @param lhs Left operand
/// @param rhs Right operand
/// @return `lhs != rhs`
template <typename T, std::size_t W, std::size_t N>
inline constexpr bool operator!=(const bitpacker<T, W, N>& lhs,
                                 const bitpacker<T, W, N>& rhs) noexcept {
  using type_dispatcher = typename bitpacker<T, W, N>::type_dispatcher;

  for (std::size_t i = 0; i < type_dispatcher::word_count; ++i) {
    if (lhs.data_[i] != rhs.data_[i]) return true;
  }

  return false;
}

/// @brief Lexicographically compares two containers
/// @tparam T I/O value type (signed/unsigned integral or enumeration type)
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
/// @param lhs Left operand
/// @param rhs Right operand
/// @return `lhs < rhs`
template <typename T, std::size_t W, std::size_t N>
inline constexpr bool operator<(const bitpacker<T, W, N>& lhs,
                                const bitpacker<T, W, N>& rhs) noexcept {
  using type_dispatcher = typename bitpacker<T, W, N>::type_dispatcher;

  for (std::size_t i = 0; i < type_dispatcher::word_count; ++i) {
    if (lhs.data_[i] != rhs.data_[i]) return lhs.data_[i] < rhs.data_[i];
  }

  return false;
}

/// @brief Lexicographically compares two containers
/// @tparam T I/O value type (signed/unsigned integral or enumeration type)
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
/// @param lhs Left operand
/// @param rhs Right operand
/// @return `lhs <= rhs`
template <typename T, std::size_t W, std::size_t N>
inline constexpr bool operator<=(const bitpacker<T, W, N>& lhs,
                                 const bitpacker<T, W, N>& rhs) noexcept {
  using type_dispatcher = typename bitpacker<T, W, N>::type_dispatcher;

  for (std::size_t i = 0; i < type_dispatcher::word_count; ++i) {
    if (lhs.data_[i] != rhs.data_[i]) return lhs.data_[i] < rhs.data_[i];
  }

  return true;
}

/// @brief Lexicographically compares two containers
/// @tparam T I/O value type (signed/unsigned integral or enumeration type)
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
/// @param lhs Left operand
/// @param rhs Right operand
/// @return `lhs > rhs`
template <typename T, std::size_t W, std::size_t N>
inline constexpr bool operator>(const bitpacker<T, W, N>& lhs,
                                const bitpacker<T, W, N>& rhs) noexcept {
  using type_dispatcher = typename bitpacker<T, W, N>::type_dispatcher;

  for (std::size_t i = 0; i < type_dispatcher::word_count; ++i) {
    if (lhs.data_[i] != rhs.data_[i]) return lhs.data_[i] > rhs.data_[i];
  }

  return false;
}

/// @brief Lexicographically compares two containers
/// @tparam T I/O value type (signed/unsigned integral or enumeration type)
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
/// @param lhs Left operand
/// @param rhs Right operand
/// @return `lhs >= rhs`
template <typename T, std::size_t W, std::size_t N>
inline constexpr bool operator>=(const bitpacker<T, W, N>& lhs,
                                 const bitpacker<T, W, N>& rhs) noexcept {
  using type_dispatcher = typename bitpacker<T, W, N>::type_dispatcher;

  for (std::size_t i = 0; i < type_dispatcher::word_count; ++i) {
    if (lhs.data_[i] != rhs.data_[i]) return lhs.data_[i] > rhs.data_[i];
  }

  return true;
}

/// @brief Signed `bitpacker<T, W, N>` with automatic I/O-type deduction
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
template <std::size_t W, std::size_t N>
using ibitpacker = bitpacker<impl::fit_signed<W>, W, N>;

/// @brief Unsigned `bitpacker<T, W, N>` with automatic I/O-type deduction
/// @tparam W Bit width of packed values
/// @tparam N Packed-value capacity
template <std::size_t W, std::size_t N>
using ubitpacker = bitpacker<impl::fit_unsigned<W>, W, N>;

}  // namespace bp3k

#endif  // !_BITPACKER3000_H_
