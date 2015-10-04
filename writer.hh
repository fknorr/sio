#pragma once

#include <type_traits>
#include <utility>
#include <memory>
#include <string>
#include <initializer_list>
#include "bitfield.hh"


namespace std {
    template<typename Char, typename Traits>
    class basic_streambuf;

    class ios_base;
    class locale;

    template<typename Char, typename OutputIt>
    class num_put;

    template<typename Char, typename Traits>
    class ostreambuf_iterator;

    template<typename Char>
    struct char_traits;
}


namespace sio {


class formatter {};

template<typename T>
using is_formatter = std::is_base_of<formatter, std::decay_t<T>>;

template<typename Implementation>
constexpr auto make_formatter(Implementation &&impl)
        noexcept(noexcept(Implementation(std::forward<Implementation>(impl)))) {
    class formatter_impl: public formatter, public std::decay_t<Implementation> {
    public:
        constexpr formatter_impl(Implementation &&impl)
            : Implementation(std::forward<Implementation>(impl)) {
        }
    };
    return formatter_impl(std::forward<Implementation>(impl));
};


template<typename Enum>
using enum_name_list = std::pair<const char*, std::initializer_list<std::pair<Enum, const char*>>>;

template<typename Enum>
struct enum_names {
    enum_name_list<Enum> operator()() const {
        return { "enum", {} };
    }
};


enum class fmt {
    oct         = 1 << 0,
    hex         = 1 << 1,
    sci         = 1 << 2,
    fixed       = 1 << 3,
    show_base   = 1 << 4,
    show_point  = 1 << 5,
    show_sign   = 1 << 6,
    uppercase   = 1 << 7
};

template<>
struct is_bit_enum<fmt>: public std::true_type {};

template<>
struct enum_names<fmt> {
    enum_name_list<fmt> operator()() const {
        return { "sio::fmt::", {
            { fmt::oct, "oct" }, { fmt::hex, "hex" }, { fmt::sci, "sci" }, { fmt::fixed, "fixed" },
            { fmt::show_base, "show_base" }, { fmt::show_point, "show_point" },
            { fmt::show_sign, "show_sign" }, { fmt::uppercase, "uppercase" }
        } };
    }
};


enum class line_ending {
    cr,
    lf,
    crlf
};

template<>
struct enum_names<line_ending> {
    enum_name_list<line_ending> operator()() const {
        return { "sio::line_ending::", {
            { line_ending::cr, "cr" }, { line_ending::lf, "lf" }, { line_ending::crlf, "crlf" }
        } };
    }
};


class writeable {
public:
    class ios_cache {
    private:
        struct ref_set {
            using streambuf_type = std::basic_streambuf<char, std::char_traits<char>>;
            using num_put_type = std::num_put<char, std::ostreambuf_iterator<char,
                    std::char_traits<char>>>;

            std::unique_ptr<std::ios_base> ios_base;
            std::unique_ptr<streambuf_type> streambuf;
            const std::locale *locale;
            const num_put_type *num_put;
        };
        std::unique_ptr<ref_set> m_refs;

        ref_set &refs() {
            if (!m_refs) {
                m_refs = std::make_unique<ref_set>();
            }
            return *m_refs;
        }

        ref_set &refs(const std::locale &locale) {
            auto &r = refs();
            if (r.locale != &locale) {
                r.num_put = nullptr;
            }
            r.locale = &locale;
            return r;
        }

        static std::unique_ptr<std::ios_base> create_ios_base();
        static std::unique_ptr<std::streambuf> create_streambuf();

        template<typename Facet>
        static const Facet &cached_facet(Facet *&ptr, const std::locale &locale);

    public:
        template<typename Facet>
        inline const Facet &locale_facet(const std::locale &);

        std::ios_base &ios_base() {
            auto &r = refs();
            if (!r.ios_base) {
                r.ios_base = create_ios_base();
            }
            return *r.ios_base;
        }

        std::streambuf &streambuf() {
            auto &r = refs();
            if (!r.streambuf) {
                r.streambuf = create_streambuf();
            }
            return *r.streambuf;
        }
    };

    virtual ~writeable() {}

    virtual ios_cache &ios() const = 0;

    virtual const std::locale &locale() const;

    virtual sio::line_ending line_ending() const noexcept {
        return sio::line_ending::lf;
    }

    virtual void write(const char *seq, std::size_t n) = 0;
};


class writer: public writeable {
public:
    virtual ios_cache &ios() const override {
        return m_ios;
    }

    writer() noexcept {}
    writer(const writer &) = delete;
    writer(writer &&rhs) = default;

    writer &operator=(const writer&) = delete;
    writer &operator=(writer &&) = default;

private:
    mutable ios_cache m_ios;
};


template<typename T>
using is_writeable = std::is_base_of<writeable, std::decay_t<T>>;


class buffered {};

template<typename T>
using is_buffered = std::is_base_of<buffered, std::decay_t<T>>;

template<typename T>
using is_buffered_writeable = std::integral_constant<bool, is_writeable<T>{} && is_buffered<T>{}>;


class format_mod {};

template<typename T>
using is_format_mod = std::is_base_of<format_mod, std::decay_t<T>>;

class format_mod_hook {};

template<typename T>
using is_format_mod_hook = std::is_base_of<format_mod_hook, std::decay_t<T>>;

template<typename T>
using is_free_format_mod = std::integral_constant<bool,
        is_format_mod<std::decay_t<T>>{} && !is_format_mod_hook<std::decay_t<T>>{}>;



template<typename Writeable, typename FormatMod,
        std::enable_if_t<is_writeable<Writeable>{} && is_format_mod<FormatMod>{}, int> = 0>
auto
operator<<(Writeable &&w, const FormatMod &mod) {
    return mod.bind(w);
}


template<typename FormatMod, typename Next,
        std::enable_if_t<is_free_format_mod<FormatMod>{}, int> = 0>
decltype(auto)
operator<<(const FormatMod &w, const Next &next) {
    write(w, next);
    return w.base();
}


template<typename Writeable, typename Next,
        std::enable_if_t<is_writeable<Writeable>{} && !is_free_format_mod<Writeable>{}, int> = 0>
decltype(auto)
operator<<(Writeable &&w, const Next &next) {
    write(w, next);
    return std::forward<Writeable>(w);
}


template<typename Writeable, typename Formatter,
        std::enable_if_t<is_formatter<Formatter>{}, int> = 0>
void
write(Writeable &w, const Formatter &fmt) {
    fmt(w);
}


template<typename FormatMod, std::enable_if_t<is_free_format_mod<FormatMod>{}, int> = 0>
auto
begin(FormatMod &&mod) {
    class hook_impl: public format_mod_hook, public std::decay_t<FormatMod> {
        public:
            hook_impl(FormatMod &&mod)
                : FormatMod(std::forward<FormatMod>(mod)) {
            }
    };
    return hook_impl(std::forward<FormatMod>(mod));
}


class end_t {} extern end;

template<typename WriterModHook, std::enable_if_t<is_format_mod_hook<WriterModHook>{}, int> = 0>
decltype(auto)
operator<<(WriterModHook &&w, end_t) {
    return w.parent();
}


class nl_t {} extern nl;

template<typename Writeable>
void
write(Writeable &w, nl_t) {
    switch (w.line_ending()) {
        case line_ending::cr: write(w, "\r"); break;
        case line_ending::lf: write(w, "\n"); break;
        case line_ending::crlf: write(w, "\r\n"); break;
    }
}


class flush_t {} extern flush;

template<typename BufferedWriteable,
        std::enable_if_t<is_buffered_writeable<BufferedWriteable>{}, int> = 0>
void
write(BufferedWriteable &w, flush_t) {
    w.flush();
}



template<typename Writeable, std::size_t N>
void
write(Writeable &w, const char (&literal)[N]) {
    w.write(literal, N-1);
}


template<typename Number, std::enable_if_t<std::is_arithmetic<Number>{}
        || std::is_same<Number, bool>{} || std::is_same<Number, const void*>{}, int> = 0>
void
write(writeable &w, const Number &v, bitfield<fmt> flags = {}, unsigned precision = 6);


template<typename Number, std::enable_if_t<std::is_arithmetic<Number>{}
        || std::is_same<Number, bool>{} || std::is_same<Number, const void*>{}, int> = 0>
auto
format(const Number &v, bitfield<fmt> flags, unsigned precision = 6) {
    return make_formatter([=](auto &w) {
        write(w, v, flags, precision);
    });
}


template<typename Ref>
class basic_string_writer final: public writer, public buffered {
public:
    using ref_type = Ref;

    basic_string_writer(Ref str)
        : m_string(&str) {
    }

    basic_string_writer(basic_string_writer &&other) {
        other.flush();
        m_string = other.m_string;
    }

    ~basic_string_writer() noexcept {
        try { flush(); } catch(...) {}
    }

    virtual void write(const char *seq, std::size_t n) override {
        if (spos + n > sz) {
            flush();
        }
        if (n > sz) {
            m_string->append(seq, seq+n);
        } else {
            std::copy(seq, seq+n, scratch+spos);
            spos += n;
        }
    }

    void flush() {
        if (spos) {
            m_string->append(scratch, scratch+spos);
            spos = 0;
        }
    }

    Ref string() {
        flush();
        return std::forward<Ref>(*m_string);
    }

private:
    static constexpr std::size_t sz = 100;
    std::string *m_string;
    char scratch[sz];
    std::size_t spos = 0;
};

using string_writer = basic_string_writer<std::string&>;
using rvalue_string_writer = basic_string_writer<std::string&&>;


template<typename Writeable>
void
write(Writeable &w, const std::string &str) {
    w.write(str.c_str(), str.length());
}


class ret_t {} extern ret;

template<typename Writeable, std::enable_if_t<
        std::is_same<std::decay_t<Writeable>, string_writer>{}
        || std::is_same<std::decay_t<Writeable>, rvalue_string_writer>{}, int> = 0>
auto
operator<<(Writeable &&w, ret_t) {
    return std::forward<typename Writeable::ref_type>(w.string());
}


template<typename Writer, typename Enum,
         std::enable_if_t<std::is_enum<Enum>{}, int> = 0>
void
write(Writer &&w, Enum value) {
    auto map = enum_names<Enum>{}();
    write(std::forward<Writer>(w), map.first);

    const char *name = nullptr;
    for (auto &pair : map.second) {
        if (pair.first == value) {
            name = pair.second;
            break;
        }
    }
    if (name) {
        write(std::forward<Writer>(w), name);
    } else {
        write(std::forward<Writer>(w), "<");
        write(std::forward<Writer>(w), static_cast<std::underlying_type_t<Enum>>(value));
        write(std::forward<Writer>(w), ">");
    }
}


namespace ops {

template<typename T>
auto
operator<<(std::string &str, T &&chain) {
    return basic_string_writer<std::string&>(str) << std::forward<T>(chain);
}

template<typename T>
auto
operator<<(std::string &&str, T &&chain) {
    return basic_string_writer<std::string&&>(std::move(str)) << std::forward<T>(chain);
}

} // namespace ops


} // namespace sio
