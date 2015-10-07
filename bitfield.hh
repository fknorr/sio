#pragma once

#include <type_traits>
#include <limits>
#include <utility>


namespace sio {


template<typename T>
struct is_bit_enum: public std::false_type {};


template<typename Enum, std::enable_if_t<std::is_enum<Enum>{}, int> = 0>
class bitfield {
public:
    using integer = std::underlying_type_t<Enum>;

private:
    integer bits;

public:
    constexpr bitfield() noexcept: bits(0) {}

    constexpr bitfield(Enum bit) noexcept: bits(static_cast<integer>(bit)) {}

    explicit constexpr bitfield(integer value) noexcept: bits(value) {}

    bitfield &operator|=(bitfield rhs) noexcept {
        bits |= rhs.bits;
        return *this;
    }

    bitfield &operator&=(bitfield rhs) noexcept {
        bits &= rhs.bits;
        return *this;
    }

    bitfield &operator^=(bitfield rhs) noexcept {
        bits ^= rhs.bits;
        return *this;
    }

    constexpr bitfield operator|(bitfield rhs) const noexcept {
        return bitfield { bits | rhs.bits };
    }

    constexpr bitfield operator&(bitfield rhs) const noexcept {
        return bitfield { bits & rhs.bits };
    }

    constexpr bitfield operator^(bitfield rhs) const noexcept {
        return bitfield { bits ^ rhs.bits };
    }

    constexpr bitfield operator~() const noexcept {
        return bitfield { ~bits };
    }

    constexpr operator bool() const noexcept {
        return !!bits;
    }

    constexpr bool operator==(bitfield rhs) const noexcept {
        return bits == rhs.bits;
    }

    constexpr bool operator!=(bitfield rhs) const noexcept {
        return bits != rhs.bits;
    }
};


template<typename Writer, typename Enum>
void
write(Writer &&w, bitfield<Enum> field) {
    bool first = true;
    for (int i = std::numeric_limits<typename bitfield<Enum>::integer>::digits-1; i >= 0; --i) {
        Enum bit = static_cast<Enum>(1 << i);
        if (field & bit) {
            if (!first) {
                write(std::forward<Writer>(w), " | ");
            } else {
                first = false;
            }
            write(std::forward<Writer>(w), bit);
        }
    }
}


} // namespace sio


template<typename Enum,
         std::enable_if_t<sio::is_bit_enum<Enum>{}, int> = 0>
sio::bitfield<Enum> operator|(Enum lhs, sio::bitfield<decltype(lhs)> rhs) {
    return rhs | lhs;
}


template<typename Enum,
         std::enable_if_t<sio::is_bit_enum<Enum>{}, int> = 0>
sio::bitfield<Enum> operator&(Enum lhs, sio::bitfield<decltype(lhs)> rhs) {
    return rhs & lhs;
}


template<typename Enum,
         std::enable_if_t<sio::is_bit_enum<Enum>{}, int> = 0>
sio::bitfield<Enum> operator^(Enum lhs, sio::bitfield<decltype(lhs)> rhs) {
    return rhs ^ lhs;
}


template<typename Enum,
         std::enable_if_t<sio::is_bit_enum<Enum>{}, int> = 0>
sio::bitfield<Enum> operator~(Enum lhs) {
    return ~sio::bitfield<Enum>(lhs);
}
