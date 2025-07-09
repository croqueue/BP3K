#ifndef _BITPACKER3000_H_
#define _BITPACKER3000_H_

#include <cstddef>
#include <cstdint>
#include <type_traits>

//////// DECLARATIONS ////////

namespace bp3k::impl {

template <typename T>
inline constexpr std::size_t bitwidth() noexcept;

template <typename IntType>
inline constexpr IntType bool_as(bool x) noexcept;

template <typename UnsignedType>
inline constexpr UnsignedType pow2log2(UnsignedType pow2) noexcept;

template <typename UnsignedType>
inline constexpr bool is_pow2(UnsignedType x) noexcept;

template <std::size_t Width>
inline constexpr std::size_t packability() noexcept;

template <std::size_t Width, std::size_t N>
inline constexpr std::size_t data_size() noexcept;

template <typename ValueType, std::size_t Width>
inline constexpr ValueType extract_value(std::uintmax_t *data, std::size_t pos) noexcept;

template <typename ValueType, std::size_t Width>
inline constexpr void embed_value(std::uintmax_t *data, std::size_t pos, ValueType x) noexcept;

template <typename ValueType, std::size_t Width>
inline constexpr std::uintmax_t dehydrate(ValueType x) noexcept;

template <typename ValueType, std::size_t Width>
inline constexpr ValueType rehydrate(std::uintmax_t word, std::size_t offset) noexcept;

template <typename ValueType, std::size_t Width>
inline constexpr std::size_t locate_word(std::size_t pos) noexcept;

template <typename ValueType, std::size_t Width>
inline constexpr std::size_t locate_offset(std::size_t pos) noexcept;

template <typename std::size_t Width>
inline constexpr std::uintmax_t value_clear_mask(std::size_t offset) noexcept;

template <typename UValueType, std::size_t Width>
inline constexpr void fill_word(std::uintmax_t& word, UValueType value);

template <typename ValueType, std::size_t Width>
constexpr ValueType ValueMask = ~((ValueType)-1 << Width);

template <typename ValueType, std::size_t Width>
constexpr ValueType SignMask = (ValueType)1 << (Width - 1);

template <typename ValueType, std::size_t Width>
constexpr ValueType SignExtendBits = ~ValueMask<ValueType, Width>;

} // namespace bp3k::impl

namespace bp3k {

template <typename TValue, std::size_t Width, std::size_t N>
class IntegralPack final {
  friend class Iterator;
  friend class CIterator;
  friend class RIterator;
  friend class CRIterator;

  constexpr std::size_t WordWidth = impl::bitwidth<std::uintmax_t>();
  constexpr std::size_t Packability = impl::packability<Width>();
  constexpr std::size_t WordBitsInUse = Width * Packability;
  constexpr std::size_t EndWordValCount = N % Packability ? N % Packability : Packability;
  constexpr std::size_t EndWordWasteBits = WordWidth - (EndWordValCount * Width);
  constexpr std::size_t PartialWordFlag = impl::bool_as<std::size_t>(EndWordValCount < Packability);
  constexpr std::size_t BufSize = (N / Packability) + PartialWordFlag;
  constexpr std::size_t MaxSize = BufSize * Packability;

  std::uintmax_t data_[BufSize] {};

  using ValueType = std::is_enum<TValue>::value ? typename std::underlying_type<TValue>::type : TValue;
  using UValueType = std::make_unsigned<ValueType>::type;

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

    inline constexpr ItemReference& operator=(UValueType x) noexcept
    {
      impl::embed_value<UValueType, Width>(this->data_ptr_, this->pos_, x);
      return *this;
    }

    inline constexpr ItemReference& operator=(const ItemReference& y) noexcept
    {
      auto value = impl::extract_value(y->data_ptr_, y->pos_);
      impl::embed_value<UValueType, Width>(this->data_ptr_, this->pos_, value);
      return *this;
    }

    inline constexpr operator UValueType() const noexcept
    {
      return impl::extract_value<ValueType, ValueWidth>(this->data_ptr_, this->pos_);
    }
  };

  class Iterator final {
    IntegralPack *pack_ptr_;
    std::ptrdiff_t pos_;

    inline constexpr Iterator(IntegralPack *pack_ptr, std::ptrdiff_t pos) noexcept
      : pack_ptr_(pack_ptr)
      , pos_(pos)
    {
    }
  
  public:
    Iterator(const Iterator&) = default;
    ~Iterator() = default;

    inline constexpr ItemReference operator*() const noexcept
    {
      return ItemReference(this->pack_ptr_->data_, this->pos_);
    }

    inline constexpr ValueType operator*() const noexcept
    {
      return impl::extract_value<ValueType, ValueWidth>(this->pack_ptr_->data_, this->pos_);
    }

    inline constexpr Iterator& operator++() noexcept
    {
      ++this->pos_;
      return *this;
    }

    inline constexpr Iterator& operator--() noexcept
    {
      --this->pos_;
      return *this;
    }

    inline constexpr Iterator operator++(int) noexcept
    {
      Iterator result(*this);
      ++this->pos_;
      return result;
    }

    inline constexpr Iterator operator--(int) noexcept
    {
      Iterator result(*this);
      --this->pos_;
      return result;
    }

    inline constexpr Iterator operator+(std::ptrdiff_t n) const noexcept
    {
      return Iterator(this->pack_ptr_->data_, this->pos_ + n);
    }

    inline constexpr Iterator operator-(std::ptrdiff_t n) const noexcept
    {
      return Iterator(this->pack_ptr_->data_, this->pos_ - n);
    }

    inline constexpr Iterator& operator+=(std::ptrdiff_t n) noexcept
    {
      this->pos_ += n;
      return *this;
    }

    inline constexpr Iterator& operator-=(std::ptrdiff_t n) noexcept
    {
      this->pos_ -= n;
      return *this;
    }

    inline constexpr bool operator==(const Iterator& rhs) const noexcept
    {
      return this->pos_ == rhs.pos_;
    }

    inline constexpr bool operator!=(const Iterator& rhs) const noexcept
    {
      return this->pos_ != rhs.pos_;
    }

    inline constexpr bool operator<(const Iterator& rhs) const noexcept
    {
      return this->pos_ < rhs.pos_;
    }

    inline constexpr bool operator<=(const Iterator& rhs) const noexcept
    {
      return this->pos_ <= rhs.pos_;
    }
    
    inline constexpr bool operator>(const Iterator& rhs) const noexcept
    {
      return this->pos_ > rhs.pos_;
    }

    inline constexpr bool operator>=(const Iterator& rhs) const noexcept
    {
      return this->pos_ >= rhs.pos_;
    }
  };

  class CIterator final {
    const IntegralPack *pack_ptr_;
    std::ptrdiff_t pos_;

    inline constexpr CIterator(const IntegralPack *pack_ptr, std::ptrdiff_t pos) noexcept
      : pack_ptr_(pack_ptr)
      , pos_(pos)
    {
    }
  
  public:
    CIterator(const CIterator&) = default;
    ~CIterator() = default;

    inline constexpr ValueType operator*() const noexcept
    {
      return impl::extract_value<UValueType, Width>(this->pack_ptr_->data_, this->pos_);
    }

    inline constexpr CIterator& operator++() noexcept
    {
      ++this->pos_;
      return *this;
    }

    inline constexpr CIterator& operator--() noexcept
    {
      --this->pos_;
      return *this;
    }

    inline constexpr CIterator operator++(int) noexcept
    {
      CIterator result(*this);
      ++this->pos_;
      return result;
    }

    inline constexpr CIterator operator--(int) noexcept
    {
      CIterator result(*this);
      --this->pos_;
      return result;
    }

    inline constexpr CIterator operator+(std::ptrdiff_t n) const noexcept
    {
      return CIterator(this->pack_ptr_->data_, this->pos_ + n);
    }

    inline constexpr CIterator operator-(std::ptrdiff_t n) const noexcept
    {
      return CIterator(this->pack_ptr_->data_, this->pos_ - n);
    }

    inline constexpr CIterator& operator+=(std::ptrdiff_t n) noexcept
    {
      this->pos_ += n;
      return *this;
    }

    inline constexpr CIterator& operator-=(std::ptrdiff_t n) noexcept
    {
      this->pos_ -= n;
      return *this;
    }

    inline constexpr bool operator==(const CIterator& rhs) const noexcept
    {
      return this->pos_ == rhs.pos_;
    }

    inline constexpr bool operator!=(const CIterator& rhs) const noexcept
    {
      return this->pos_ != rhs.pos_;
    }

    inline constexpr bool operator<(const CIterator& rhs) const noexcept
    {
      return this->pos_ < rhs.pos_;
    }

    inline constexpr bool operator<=(const CIterator& rhs) const noexcept
    {
      return this->pos_ <= rhs.pos_;
    }
    
    inline constexpr bool operator>(const CIterator& rhs) const noexcept
    {
      return this->pos_ > rhs.pos_;
    }

    inline constexpr bool operator>=(const CIterator& rhs) const noexcept
    {
      return this->pos_ >= rhs.pos_;
    }
  };

  class RIterator final {
    IntegralPack *pack_ptr_;
    std::ptrdiff_t pos_;

    inline constexpr RIterator(IntegralPack *pack_ptr, std::ptrdiff_t pos) noexcept
      : pack_ptr_(pack_ptr)
      , pos_(pos)
    {
    }
  
  public:
    RIterator(const RIterator&) = default;
    ~RIterator() = default;

    inline constexpr ItemReference operator*() const noexcept
    {
      return ItemReference(this->pack_ptr_->data_, this->pos_);
    }

    inline constexpr ValueType operator*() const noexcept
    {
      return impl::extract_value<ValueType, ValueWidth>(this->pack_ptr_->data_, this->pos_);
    }

    inline constexpr RIterator& operator++() noexcept
    {
      --this->pos_;
      return *this;
    }

    inline constexpr RIterator& operator--() noexcept
    {
      ++this->pos_;
      return *this;
    }

    inline constexpr RIterator operator++(int) noexcept
    {
      RIterator result(*this);
      --this->pos_;
      return result;
    }

    inline constexpr RIterator operator--(int) noexcept
    {
      RIterator result(*this);
      ++this->pos_;
    }

    inline constexpr RIterator operator+(std::ptrdiff_t n) const noexcept
    {
      return RIterator(this->pack_ptr_->data_, this->pos_ - n);
    }

    inline constexpr RIterator operator-(std::ptrdiff_t n) const noexcept
    {
      return RIterator(this->pack_ptr_->data_, this->pos_ + n);
    }

    inline constexpr RIterator& operator+=(std::ptrdiff_t n) noexcept
    {
      this->pos_ -= n;
      return *this;
    }

    inline constexpr RIterator& operator-=(std::ptrdiff_t n) noexcept
    {
      this->pos_ += n;
      return *this;
    }

    inline constexpr bool operator==(const RIterator& rhs) const noexcept
    {
      return this->pos_ == rhs.pos_;
    }

    inline constexpr bool operator!=(const RIterator& rhs) const noexcept
    {
      return this->pos_ != rhs.pos_;
    }

    inline constexpr bool operator<(const RIterator& rhs) const noexcept
    {
      return this->pos_ >= rhs.pos_;
    }

    inline constexpr bool operator<=(const RIterator& rhs) const noexcept
    {
      return this->pos_ > rhs.pos_;
    }

    inline constexpr bool operator>(const RIterator& rhs) const noexcept
    {
      return this->pos_ <= rhs.pos_;
    }

    inline constexpr bool operator>=(const RIterator& rhs) const noexcept
    {
      return this->pos_ < rhs.pos_;
    }
  };

  class CRIterator final {
    const IntegralPack *pack_ptr_;
    std::ptrdiff_t pos_;

    inline constexpr CRIterator(const IntegralPack *pack_ptr, std::ptrdiff_t pos) noexcept
      : pack_ptr_(pack_ptr)
      , pos_(pos)
    {
    }
  
  public:
    CRIterator(const CRIterator&) = default;
    ~CRIterator() = default;

    inline constexpr ValueType operator*() const noexcept
    {
      return impl::extract_value<UValueType, Width>(this->pack_ptr_->data_, this->pos_);
    }

    inline constexpr CRIterator& operator++() noexcept
    {
      --this->pos_;
      return *this;
    }

    inline constexpr CRIterator& operator--() noexcept
    {
      ++this->pos_;
      return *this;
    }

    inline constexpr CRIterator operator++(int) noexcept
    {
      CRIterator result(*this);
      --this->pos_;
      return result;
    }

    inline constexpr CRIterator operator--(int) noexcept
    {
      CRIterator result(*this);
      ++this->pos_;
    }

    inline constexpr CRIterator operator+(std::ptrdiff_t n) const noexcept
    {
      return CRIterator(this->pack_ptr_->data_, this->pos_ - n);
    }

    inline constexpr CRIterator operator-(std::ptrdiff_t n) const noexcept
    {
      return CRIterator(this->pack_ptr_->data_, this->pos_ + n);
    }

    inline constexpr CRIterator& operator+=(std::ptrdiff_t n) noexcept
    {
      this->pos_ -= n;
      return *this;
    }

    inline constexpr CRIterator& operator-=(std::ptrdiff_t n) noexcept
    {
      this->pos_ += n;
      return *this;
    }

    inline constexpr bool operator==(const CRIterator& rhs) const noexcept
    {
      return this->pos_ == rhs.pos_;
    }

    inline constexpr bool operator!=(const CRIterator& rhs) const noexcept
    {
      return this->pos_ != rhs.pos_;
    }

    inline constexpr bool operator<(const CRIterator& rhs) const noexcept
    {
      return this->pos_ >= rhs.pos_;
    }

    inline constexpr bool operator<=(const CRIterator& rhs) const noexcept
    {
      return this->pos_ > rhs.pos_;
    }

    inline constexpr bool operator>(const CRIterator& rhs) const noexcept
    {
      return this->pos_ <= rhs.pos_;
    }

    inline constexpr bool operator>=(const CRIterator& rhs) const noexcept
    {
      return this->pos_ < rhs.pos_;
    }
  };

  using value_type = TValue;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = ItemReference;
  using const_reference = const ItemReference;
  using iterator = Iterator;
  using const_iterator = CIterator;
  using reverse_iterator = RIterator;
  using const_reverse_iterator = CRIterator;

  IntegralPack() = default;
  inline constexpr IntegralPack(ValueType x) noexcept
  {
    for (std::size_t i = 0; i < BufSize; ++i)
      impl::fill_word<ValueType, Width>(this->data_[i], x);
    
    this->data_[BufSize - 1] &= (std::uintmax_t)-1 >> EndWordWasteBits;
  }

  inline constexpr reference operator[](size_type pos) noexcept
  {
    return reference(this->data_, pos);
  }

  inline constexpr value_type operator[](size_type pos) const noexcept
  {
    return impl::extract_value<UValueType, Width>(this->data_, pos);
  }

  inline constexpr reference front() noexcept
  {
    return reference(this->data_, 0);
  }

  inline constexpr const_reference front() const noexcept
  {
    return const_refereturn const_reverse_iterator(nullptr, -1);rence(this->data_, 0);
  }

  inline constexpr reference back() noexcept
  {
    return reference(this->data_, N - 1);
  }

  inline constexpr const_reference back() const noexcept
  {
    return const_reference(this->data_, N - 1);
  }

  inline constexpr std::uintmax_t *data() noexcept
  {
    return this->data_;
  }

  inline constexpr const std::uintmax_t *data() const noexcept
  {
    return this->data_;
  }

  inline constexpr iterator begin() noexcept
  {
    return iterator(this, 0);
  }

  inline constexpr const_iterator begin() const noexcept
  {
    return const_iterator(this, 0);
  }

  inline constexpr const_iterator cbegin() const noexcept
  {
    return const_iterator(this, 0);
  }

  inline constexpr iterator end() noexcept
  {
    return iterator(nullptr, static_cast<std::ptrdiff_t>(N - 1));
  }

  inline constexpr const_iterator end() const noexcept
  {
    return const_iterator(nullptr, static_cast<std::ptrdiff_t>(N - 1));
  }

  inline constexpr const_iterator cend() const noexcept
  {
    return const_iterator(nullptr, static_cast<std::ptrdiff_t>(N - 1));
  }

  inline constexpr reverse_iterator rbegin() noexcept
  {
    return reverse_iterator(this, static_cast<std::ptrdiff_t>(N - 1));
  }

  inline constexpr const_reverse_iterator rbegin() const noexcept
  {
    return const_reverse_iterator(this, static_cast<std::ptrdiff_t>(N - 1));
  }

  inline constexpr const_reverse_iterator crbegin() const noexcept
  {
    return const_reverse_iterator(this, static_cast<std::ptrdiff_t>(N - 1));
  }

  inline constexpr reverse_iterator rend() noexcept
  {
    return reverse_iterator(nullptr, -1);
  }

  inline constexpr const_reverse_iterator rend() const noexcept
  {
    return const_reverse_iterator(nullptr, -1);
  }

  inline constexpr const_reverse_iterator crend() const noexcept
  {
    return const_reverse_iterator(nullptr, -1);
  }

  inline constexpr size_type size() const noexcept
  {
    return N;
  }

  inline constexpr size_type max_size() const noexcept
  {
    return MaxSize;
  }

  inline constexpr void fill(const ValueType& x) noexcept
  {
    for (std::size_t i = 0; i < BufSize; ++i)
      impl::fill_word<ValueType, Width>(this->data_[i], x);
    
    this->data_[BufSize - 1] &= (std::uintmax_t)-1 << EndWordWasteBits;
  }

  inline constexpr void swap(IntegralPack& y) noexcept
  {
    std::uintmax_t tmp {};

    for (std::size_t i = 0; i < BufSize; ++i) {
      tmp = this->data_[i];
      this->data_[i] = y.data_[i];
      y.data_[i] = tmp;
    }
  }
};

// template <typename EnumType, std::size_t ValueWidth, std::size_t Size>
// using EnumPack = IntegralPack<typename std::underlying_type<EnumType>::type, ValueWidth, Size>;

template <typename ValueType, std::size_t Width, std::size_t N>
inline constexpr bool operator==(const IntegralPack<ValueType, Width, N>& lhs, const IntegralPack<ValueType, Width, N>& rhs) noexcept
{
  for (std::size_t i = 0; i < BufSize; ++i) {
    if (lhs.data_[i] != rhs.data_[i])
      return false;
  }
  
  return true;
}

template <typename ValueType, std::size_t Width, std::size_t N>
inline constexpr bool operator!=(const IntegralPack<ValueType, Width, N>& lhs, const IntegralPack<ValueType, Width, N>& rhs) noexcept
{
  for (std::size_t i = 0; i < BufSize; ++i) {
    if (lhs.data_[i] != rhs.data_[i])
      return true;
  }

  return false;
}

template <typename ValueType, std::size_t Width, std::size_t N>
inline constexpr bool operator<(const IntegralPack<ValueType, Width, N>& lhs, const IntegralPack<ValueType, Width, N>& rhs) noexcept
{
  for (std::size_t i = 0; i < BufSize; ++i) {
    if (lhs.data_[i] >= rhs.data_[i])
      return false;
  }

  return true;
}

template <typename ValueType, std::size_t Width, std::size_t N>
inline constexpr bool operator<=(const IntegralPack<ValueType, Width, N>& lhs, const IntegralPack<ValueType, Width, N>& rhs) noexcept
{
  for (std::size_t i = 0; i < BufSize; ++i) {
    if (lhs.data_[i] > rhs.data_[i])
      return false;
  }

  return true;
}

template <typename ValueType, std::size_t Width, std::size_t N>
inline constexpr bool operator>(const IntegralPack<ValueType, Width, N>& lhs, const IntegralPack<ValueType, Width, N>& rhs) noexcept
{
  for (std::size_t i = 0; i < BufSize; ++i) {
    if (lhs.data_[i] <= rhs.data_[i])
      return false;
  }

  return true;
}

template <typename ValueType, std::size_t Width, std::size_t N>
inline constexpr bool operator>=(const IntegralPack<ValueType, Width, N>& lhs, const IntegralPack<ValueType, Width, N>& rhs) noexcept
{
  for (std::size_t i = 0; i < BufSize; ++i) {
    if (lhs.data_[i] < rhs.data_[i])
      return false;
  }

  return true;
}

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

template <typename UnsignedType>
constexpr bool is_pow2(UnsignedType x) noexcept
{
  return !(x & (x - 1));
}

template <std::size_t Width>
constexpr std::size_t packability() noexcept
{
  return bitwidth<uintmax_t>() / Width;
}

template <typename ValueType, std::size_t Width>
constexpr ValueType extract_value(std::uintmax_t *data, std::size_t pos) noexcept
{
  using UValueType = typename std::make_unsigned<ValueType>::type;

  constexpr auto sign_mask = SignMask<ValueType, ValueWidth>;
  constexpr auto sign_extend_bits = SignExtendBits<ValueType, ValueWidth>;

  std::size_t word_index = locate_word(pos);
  std::size_t offset = locate_offset<ValueType, Width>(pos);
  UValueType value = rehydrate<UValueType, Width>(data[word_index], offset);

  if constexpr (!std::is_signed<ValueType>::value)
    return value;

  constexpr auto sign_mask = SignMask<UValueType, ValueWidth>;
  constexpr auto sign_extend_bits = SignExtendBits<UValueType, ValueWidth>;

  UValueType is_negative = bool_as<UValueType>(value & sign_mask != 0);
  return static_cast<ValueType>(value | (sign_extend_bits * is_negative));
}

template <typename ValueType, std::size_t Width>
constexpr void embed_value(std::uintmax_t *data, std::size_t pos, ValueType x) noexcept
{
  std::size_t word_index = locate_word<ValueType, Width>(pos);
  std::size_t offset = locate_offset<ValueType, Width>(pos);

  data[word_index] &= value_clear_mask<Width>(offset);
  data[word_index] |= dehydrate<ValueType, Width>(x) << offset;
}

template <typename ValueType, std::size_t Width>
constexpr std::uintmax_t dehydrate(ValueType x) noexcept
{
  using UValueType = typename std::make_unsigned<ValueType>::type;

  constexpr auto value_mask = ValueMask<UValueType, ValueWidth>;

  auto value = static_cast<std::uintmax_t>(static_cast<UValueType>(x) & value_mask);
  return value;
}

template <typename ValueType, std::size_t Width>
constexpr ValueType rehydrate(std::uintmax_t word, std::size_t offset) noexcept
{
  constexpr auto value_mask = ValueMask<ValueType, ValueWidth>;
  
  return static_cast<ValueType>(word >> offset) & value_mask;
}

template <typename ValueType, std::size_t Width>
constexpr std::size_t locate_word(std::size_t pos) noexcept
{
  constexpr auto word_shift = pow2log2(bitwidth<std::uintmax_t>());

  return pos >> word_shift;
}

template <typename ValueType, std::size_t Width>
constexpr std::size_t locate_offset(std::size_t pos) noexcept
{
  if constexpr (is_pow2(Width)) {
    std::size_t val_pos = pos & (Packability - 1);
  }
  else {
    std::size_t val_pos = pos % Packability;
  }

  return WordWidth - ((val_pos + 1) * Width);
}

template <typename std::size_t Width>
constexpr std::uintmax_t value_clear_mask(std::size_t offset) noexcept
{
  return ~(ValueMask<std::uintmax_t, ValueWidth> << offset);
}

template <typename ValueType, std::size_t Width>
constexpr void fill_word(std::uintmax_t& word, ValueType value)
{
  using UValueType = typename std::make_unsigned<ValueType>::type;

  std::uintmax_t mask = dehydrate<UValueType, Width>(static_cast<UValueType>(value));
  word = 0;

  for (std::size_t i = 0; i < packability<Width>(); ++i) {
    word |= mask;
    mask <<= Width;
  }
}

} // namespace bp3k::impl


namespace bp3k {

} // namespace bp3k

#endif // !_BITPACKER3000_H_
