#pragma once

#include <initializer_list>
#include <utility>


namespace sio {


template<typename Enum>
using enum_name_list = std::pair<const char*, std::initializer_list<std::pair<Enum, const char*>>>;

template<typename Enum>
struct enum_names {
    enum_name_list<Enum> operator()() const {
        return { "enum", {} };
    }
};


} // namespace sio
