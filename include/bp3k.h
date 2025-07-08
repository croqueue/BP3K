#ifndef _BITPACKER3000_H_
#define _BITPACKER3000_H_

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>

//////// DECLARATIONS ////////

namespace bp3k::impl {

template <typename T>
inline constexpr std::size_t bitwidth() noexcept;

template <typename UnsignedType>
inline constexpr UnsignedType pow2log2(UnsignedType pow2) noexcept;

template <std::size_t ValueWidth>
inline constexpr std::size_t packability() noexcept;

template <std::size_t ValueWidth, std::size_t Size>
inline constexpr std::size_t data_size() noexcept;

template <typename ValueType, std::size_t ValueWidth>
inline constexpr ValueType extract_value(std::uintmax_t *data, std::size_t pos) noexcept;

template <typename ValueType, std::size_t ValueWidth>
inline constexpr void embed_value(std::uintmax_t *data, std::size_t pos, ValueType x) noexcept;

template <typename ValueType, std::size_t ValueWidth>
inline constexpr std::uintmax_t deflate(ValueType x) noexcept;

template <typename ValueType, std::size_t ValueWidth>
inline constexpr ValueType inflate(std::uintmax_t word, std::size_t offset) noexcept;

template <typename ValueType, std::size_t ValueWidth>
inline constexpr std::size_t locate_word(std::size_t pos) noexcept;

} // namespace bp3k::impl

namespace bp3k {

template <typename ValueType, std::size_t ValueWidth, std::size_t Size>
class IntegralPack final {
  std::uintmax_t data_[impl::data_size<ValueWidth, Size>()];

public:
  class ItemReference final {
    friend class IntegralPack;

    std::uintmax_t * const data_ptr_;
    const std::size_t pos_;

    inline constexpr ItemReference(std::uintmax_t *data_ptr, std::size_t pos) noexcept
      : data_ptr_(data_ptr)
      , pos_(pos)
    {
    }
  
  public:

    ItemReference(const ItemReference&) = default;
    ~ItemReference() = default;

    inline constexpr ItemReference& operator=(ValueType x) noexcept
    {
      impl::embed_value<ValueType, ValueWidth>(this->data_ptr_, this->pos_, x);
      return *this;
    }

    inline constexpr ItemReference& operator=(const ItemReference& y) noexcept
    {
      auto value = impl::extract_value(y->data_ptr_, y->pos_);
      impl::embed_value<ValueType, ValueWidth>(this->data_ptr_, this->pos_, value);
      return *this;
    }

    inline constexpr operator ValueType() const noexcept
    {
      return impl::extract_value<ValueType, ValueWidth>(this->data_ptr_, this->pos_);
    }
  };

  class ItemIterator final {
    ;
  };

  class ConstItemIterator final {
    ;
  };

  using value_type = ValueType;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;

  // Note: Cannot support random access with pointers
  // using pointer = value_type*;
  // using const_pointer = const value_type*;

  using iterator = ItemIterator;
  using const_iterator = ConstItemIterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  inline constexpr ItemReference operator[](std::size_t pos) noexcept
  {
    return ItemReference(this->data_, pos);
  }

  inline constexpr ValueType operator[](std::size_t pos) const noexcept
  {
    return impl::extract_value<ValueType, ValueWidth>(this->data_, pos);
  }

  inline constexpr reference front() noexcept;
  inline constexpr const_reference front() const noexcept;
  inline constexpr reference back() noexcept;
  inline constexpr const_reference back() const noexcept;
  inline constexpr std::uintmax_t *data() noexcept;
  inline constexpr const std::uintmax_t *data() const noexcept;
  inline constexpr iterator begin() noexcept;
  inline constexpr const_iterator begin() const noexcept;
  inline constexpr const_iterator cbegin() const noexcept;
  inline constexpr iterator end() noexcept;
  inline constexpr const_iterator end() const noexcept;
  inline constexpr const_iterator cend() const noexcept;
  inline constexpr reverse_iterator rbegin() noexcept;
  inline constexpr const_reverse_iterator rbegin() const noexcept;
  inline constexpr const_reverse_iterator crbegin() const noexcept;
  inline constexpr reverse_iterator rend() noexcept;
  inline constexpr const_reverse_iterator rend() const noexcept;
  inline constexpr const_reverse_iterator crend() const noexcept;
  inline constexpr bool empty() const noexcept;
  inline constexpr size_type size() const noexcept;
  inline constexpr size_type max_size() const noexcept;
  inline constexpr void fill(const ValueType& x) noexcept;
  inline constexpr void swap(IntegralPack& y) noexcept;
};


template <typename ValueType, std::size_t ValueWidth, std::size_t Size>
inline constexpr bool operator==(const IntegralPack<ValueType, ValueWidth, Size>& lhs, const IntegralPack<ValueType, ValueWidth, Size>& rhs) noexcept;

template <typename ValueType, std::size_t ValueWidth, std::size_t Size>
inline constexpr bool operator!=(const IntegralPack<ValueType, ValueWidth, Size>& lhs, const IntegralPack<ValueType, ValueWidth, Size>& rhs) noexcept;

template <typename ValueType, std::size_t ValueWidth, std::size_t Size>
inline constexpr bool operator<(const IntegralPack<ValueType, ValueWidth, Size>& lhs, const IntegralPack<ValueType, ValueWidth, Size>& rhs) noexcept;

template <typename ValueType, std::size_t ValueWidth, std::size_t Size>
inline constexpr bool operator<=(const IntegralPack<ValueType, ValueWidth, Size>& lhs, const IntegralPack<ValueType, ValueWidth, Size>& rhs) noexcept;

template <typename ValueType, std::size_t ValueWidth, std::size_t Size>
inline constexpr bool operator>(const IntegralPack<ValueType, ValueWidth, Size>& lhs, const IntegralPack<ValueType, ValueWidth, Size>& rhs) noexcept;

template <typename ValueType, std::size_t ValueWidth, std::size_t Size>
inline constexpr bool operator>=(const IntegralPack<ValueType, ValueWidth, Size>& lhs, const IntegralPack<ValueType, ValueWidth, Size>& rhs) noexcept;

} // namespace bp3k

//////// DEFINITIONS ////////

namespace bp3k::impl {

template <typename T>
constexpr std::size_t bitwidth() noexcept
{
  return sizeof(T) << 3;
}

template <typename UnsignedType>
constexpr UnsignedType pow2log2(UnsignedType pow2) noexcept
{
  UnsignedType log2 {};

  for (; pow2 > 1; pow2 >>= 1, ++log2)
    ;
  
  return log2;
}

template <std::size_t ValueWidth>
constexpr std::size_t packability() noexcept
{
  return bitwidth<uintmax_t>() / ValueWidth;
}

template <std::size_t ValueWidth, std::size_t Size>
constexpr std::size_t data_size() noexcept
{
  return packability<ValueWidth>() / Size;
}

template <typename ValueType, std::size_t ValueWidth>
constexpr ValueType extract_value(std::uintmax_t *data, std::size_t pos) noexcept
{
  auto word = locate_word(pos);


  if constexpr (std::is_signed<ValueType>::value) {
    ;
  }
  else {
    ;
  }
}

template <typename ValueType, std::size_t ValueWidth>
constexpr void embed_value(std::uintmax_t *data, std::size_t pos, ValueType x) noexcept
{
  if constexpr (std::is_signed<ValueType>::value) {
    ;
  }
  else {
    ;
  }
}

template <typename ValueType, std::size_t ValueWidth>
constexpr std::uintmax_t deflate(ValueType x) noexcept
{
  if constexpr (std::is_signed<ValueType>::value) {
    ;
  }
  else {
    ;
  }
}

template <typename ValueType, std::size_t ValueWidth>
constexpr ValueType inflate(std::uintmax_t word, std::size_t offset) noexcept
{
  if constexpr (std::is_signed<ValueType>::value) {
    ;
  }
  else {
    ;
  }
}

template <typename ValueType, std::size_t ValueWidth>
constexpr std::size_t locate_word(std::size_t pos) noexcept
{
  if constexpr (std::is_signed<ValueType>::value) {
    ;
  }
  else {
    ;
  }
}

} // namespace bp3k::impl


namespace bp3k {

} // namespace bp3k

#endif // !_BITPACKER3000_H_
