#pragma once

#include <type_traits>
#include <utility>
#include <memory>
#include <string>
#include "../enum.hh"
#include "../bitfield.hh"


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

    size_t strlen(const char *str);
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



enum class fmt {
    oct             = 1 << 0,
    hex             = 1 << 1,
    sci             = 1 << 2,
    fixed           = 1 << 3,
    show_base       = 1 << 4,
    show_point      = 1 << 5,
    show_sign       = 1 << 6,
    uppercase       = 1 << 7,
    left            = 1 << 8,
    right           = 1 << 9,
    center          = 1 << 10,
    base_mask       = oct | hex | uppercase,
    float_mask      = sci | fixed | uppercase,
    justify_mask    = left | right | center
};

template<>
struct is_bit_enum<fmt>: public std::true_type {};

template<>
struct enum_names<fmt> {
    enum_name_list<fmt> operator()() const {
        return { "sio::fmt::", {
            { fmt::oct, "oct" }, { fmt::hex, "hex" }, { fmt::sci, "sci" }, { fmt::fixed, "fixed" },
            { fmt::show_base, "show_base" }, { fmt::show_point, "show_point" },
            { fmt::show_sign, "show_sign" }, { fmt::uppercase, "uppercase" },
            { fmt::left, "left" }, { fmt::right, "right" }, { fmt::center, "center" }
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

            ~ref_set();

            std::unique_ptr<std::ios_base> ios_base;
            std::unique_ptr<streambuf_type> streambuf;
            const std::locale *locale;
            const num_put_type *num_put;
        };
        std::unique_ptr<ref_set> m_refs;

        void create_refs();

        ref_set &refs() {
            if (!m_refs) create_refs();
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

        void create_ios_base();
        void create_streambuf();

        template<typename Facet>
        static const Facet &cached_facet(Facet *&ptr, const std::locale &locale);

    public:
        template<typename Facet>
        inline const Facet &locale_facet(const std::locale &);

        std::ios_base &ios_base() {
            auto &r = refs();
            if (!r.ios_base) create_ios_base();
            return *r.ios_base;
        }

        std::streambuf &streambuf() {
            auto &r = refs();
            if (!r.streambuf) create_streambuf();
            return *r.streambuf;
        }
    };

    virtual ~writeable() {}

protected:
    virtual ios_cache &v_ios() const = 0;

    virtual const std::locale &v_locale() const;

    virtual sio::line_ending v_line_ending() const noexcept {
        return sio::line_ending::lf;
    }

    virtual void v_write(const char *seq, std::size_t n) = 0;

    virtual bitfield<fmt> v_flags() const noexcept {
        return {};
    }

    virtual unsigned v_width() const noexcept {
        return 0;
    }

    virtual unsigned v_precision() const noexcept {
        return 6;
    }

public:
    ios_cache &ios() const {
        return v_ios();
    }

    const std::locale &locale() {
        return v_locale();
    }

    sio::line_ending line_ending() const noexcept {
        return v_line_ending();
    }

    void write(const char *seq, std::size_t n) {
        v_write(seq, n);
    }

    bitfield<fmt> flags() const noexcept {
        return v_flags();
    }

    unsigned width() const noexcept {
        return v_width();
    }

    unsigned precision() const noexcept {
        return v_precision();
    }
};


class writer: public writeable {
public:
    writer() noexcept {}
    writer(const writer &) {}
    writer(writer &&) {}

    writer &operator=(const writer&) { return *this; }
    writer &operator=(writer &&) { return *this; }

protected:
    virtual ios_cache &v_ios() const override {
        return m_ios;
    }

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


template<typename Writeable, std::size_t N>
void
write(Writeable &w, const char (&literal)[N]) {
    w.write(literal, N-1);
}


template<typename Writeable, typename ConstIntPtr,
        std::enable_if_t<std::is_same<const int*, std::remove_reference_t<ConstIntPtr>>{}, int> = 0>
void
write(Writeable& w, ConstIntPtr string) {
    w.write(string, std::strlen(string));
}


template<typename Writer, typename Value>
struct write_function_defined {
private:
    template<typename W, typename V>
    static std::true_type test(
            std::decay_t<decltype(write(std::declval<W&>(), std::declval<V>()))> *);

    template<typename W, typename V>
    static std::false_type test(...);

public:
    enum : bool { value = decltype(test<Writer, Value>(nullptr))::value };

    constexpr write_function_defined() noexcept {}
    constexpr operator bool() noexcept { return value; }
    constexpr bool operator()() noexcept { return value; }
};

template<typename Writer, typename Value,
        std::enable_if_t<write_function_defined<Writer, Value>{}, int> = 0>
void
dispatch_write(Writer &w, Value &&v) {
    write(w, std::forward<Value>(v));
}


template<typename Writer>
void
dispatch_write(Writer &&, ...) {
    static_assert(!sizeof(Writer), "Right-hand-side type must either have sio::write(Writer&, Rhs) "
            "or operator<<(std::ostream&, Rhs) defined (with sio/compat.hh included for the "
            "second option)");
}


class format_mod_base {};

template<typename T>
using is_format_mod = std::is_base_of<format_mod_base, std::decay_t<T>>;


template<typename Writeable>
class format_mod: public writeable, public format_mod_base {
public:
    using parent_type = Writeable;

    auto &bind(Writeable &w) {
        m_parent = &w;
        return *this;
    }

    Writeable &parent() noexcept {
        return *m_parent;
    }

    template<typename W = Writeable, std::enable_if_t<is_format_mod<W>{}, int> = 0>
    auto &base() noexcept {
        return m_parent->base();
    }

    template<typename W = Writeable, std::enable_if_t<!is_format_mod<W>{}, int> = 0>
    auto &base() noexcept {
        return *m_parent;
    }

protected:
    virtual writeable::ios_cache &v_ios() const override {
        return m_parent->ios();
    }

    virtual const std::locale &v_locale() const override {
        return m_parent->locale();
    }

    virtual sio::line_ending v_line_ending() const noexcept override {
        return m_parent->line_ending();
    }

    virtual void v_write(const char *seq, std::size_t n) override {
        m_parent->write(seq, n);
    }

    virtual bitfield<fmt> v_flags() const noexcept override {
        return m_parent->flags();
    }

    virtual unsigned v_width() const noexcept override {
        return m_parent->width();
    }

    virtual unsigned v_precision() const noexcept override {
        return m_parent->precision();
    }

private:
    Writeable *m_parent;
};


class format_mod_hook {};

template<typename T>
using is_format_mod_hook = std::is_base_of<format_mod_hook, std::decay_t<T>>;

template<typename T>
using is_free_format_mod = std::integral_constant<bool,
        is_format_mod<std::decay_t<T>>{} && !is_format_mod_hook<std::decay_t<T>>{}>;


class format_mod_tag {};

template<typename T>
using is_format_mod_tag = std::is_base_of<format_mod_tag, std::decay_t<T>>;


template<typename Writeable, typename FormatMod,
        std::enable_if_t<is_writeable<Writeable>{} && is_format_mod<FormatMod>{}, int> = 0>
auto
operator<<(Writeable &&w, const FormatMod &mod) {
    return mod.bind(w);
}


template<typename FormatMod, typename Next,
        std::enable_if_t<is_free_format_mod<FormatMod>{} && !is_format_mod<Next>{}
            && !is_format_mod_tag<Next>{}, int> = 0>
decltype(auto)
operator<<(FormatMod &&w, Next &&next) {
    dispatch_write(static_cast<FormatMod&>(w), std::forward<Next>(next));
    return w.base();
}


template<typename Writeable, typename Next,
        std::enable_if_t<is_writeable<Writeable>{} && !is_free_format_mod<Writeable>{}
            && !is_format_mod<Next>{} && !is_format_mod_tag<Next>{}, int> = 0>
decltype(auto)
operator<<(Writeable &&w, Next &&next) {
    dispatch_write(static_cast<Writeable&>(w), std::forward<Next>(next));
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


template<typename Writeable, typename FormatModTag,
         std::enable_if_t<is_writeable<Writeable>{} && !is_format_mod<FormatModTag>{}
            && is_format_mod_tag<FormatModTag>{}, int> = 0>
auto
operator<<(Writeable &&w, FormatModTag &&tag) {
    auto mod = tag.template create<Writeable>();
    mod.bind(w);
    return mod;
}


template<typename Writeable, fmt Flags>
class add_flag_format_mod: public format_mod<Writeable> {
public:
    auto &bind(Writeable &w) noexcept {
        format_mod<Writeable>::bind(w);
        return *this;
    }

protected:
    virtual bitfield<fmt> v_flags() const noexcept override {
        return format_mod<Writeable>::v_flags() | Flags;
    }
};

template<fmt Flags>
struct add_flag_format_mod_tag: public format_mod_tag {
    template<typename Writeable>
    constexpr auto create() const {
        return add_flag_format_mod<std::decay_t<Writeable>, Flags>{};
    }
};

extern add_flag_format_mod_tag<fmt::oct> oct;
extern add_flag_format_mod_tag<fmt::hex> hex;
extern add_flag_format_mod_tag<fmt::sci> sci;
extern add_flag_format_mod_tag<fmt::fixed> fixed;
extern add_flag_format_mod_tag<fmt::show_base> show_base;
extern add_flag_format_mod_tag<fmt::show_point> show_point;
extern add_flag_format_mod_tag<fmt::show_sign> show_sign;
extern add_flag_format_mod_tag<fmt::uppercase> uppercase;


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


template<typename Number, std::enable_if_t<std::is_arithmetic<Number>{}
        || std::is_same<Number, bool>{} || std::is_same<Number, const void*>{}, int> = 0>
void
write(writeable &w, const Number &v, bitfield<fmt> flags, unsigned precision);


template<typename Writeable, typename Number, std::enable_if_t<std::is_arithmetic<Number>{}
        || std::is_same<Number, bool>{} || std::is_same<Number, const void*>{}, int> = 0>
void
write(Writeable &w, const Number &v) {
    write(w, v, w.flags(), w.precision());
}


template<typename Number, std::enable_if_t<std::is_arithmetic<Number>{}
        || std::is_same<Number, bool>{} || std::is_same<Number, const void*>{}, int> = 0>
auto
num(const Number &v, bitfield<fmt> flags, unsigned precision = 6) {
    return make_formatter([=](auto &w) {
        write(w, v, flags, precision);
    });
}


class discarding_writer final: public writer {
protected:
    virtual void v_write(const char *, std::size_t) override {}
};

extern discarding_writer nirvana;


template<typename Ref, typename Store>
class basic_string_writer final: public writer, public buffered {
public:
    using ref_type = Ref;
    using store_type = Store;

    template<typename U = Store, std::enable_if_t<!std::is_pointer<U>{}, int> = 0>
    basic_string_writer() {}

    template<typename U = Store, std::enable_if_t<std::is_pointer<U>{}, int> = 0>
    explicit basic_string_writer(Ref str)
        : m_string(&str) {
    }

    template<typename U = Store, std::enable_if_t<!std::is_pointer<U>{}, int> = 0>
    explicit basic_string_writer(Ref str)
        : m_string(std::forward<Ref>(str)) {
    }

    basic_string_writer(const basic_string_writer &other) {
        other.flush();
        m_string = other.m_string;
    }

    basic_string_writer(basic_string_writer &&other) {
        other.flush();
        m_string = std::move(other.m_string);
    }

    ~basic_string_writer() noexcept {
        try { flush(); } catch(...) {}
    }

    void flush() const {
        if (spos) {
            str_ref().append(scratch, scratch+spos);
            spos = 0;
        }
    }

    Ref str() const {
        flush();
        return static_cast<Ref>(str_ref());
    }

protected:
    virtual void v_write(const char *seq, std::size_t n) override {
        if (spos + n > sz) {
            flush();
        }
        if (n > sz) {
            str_ref().append(seq, seq+n);
        } else {
            std::copy(seq, seq+n, scratch+spos);
            spos += n;
        }
    }

private:
    template<typename U = Store, std::enable_if_t<std::is_pointer<U>{}, int> = 0>
    std::string &str_ref() const {
        return *m_string;
    }

    template<typename U = Store, std::enable_if_t<!std::is_pointer<U>{}, int> = 0>
    std::string &str_ref() const {
        return static_cast<std::string&>(m_string);
    }

    static constexpr std::size_t sz = 100;
    mutable Store m_string;
    char scratch[sz];
    mutable std::size_t spos = 0;
};


using string_writer = basic_string_writer<const std::string&, std::string>;
using ref_string_writer = basic_string_writer<std::string&, std::string*>;
using rvalue_string_writer = basic_string_writer<std::string&&, std::string*>;


template<typename Writeable>
void
write(Writeable &w, const std::string &str) {
    w.write(str.c_str(), str.length());
}


class ret_t {} extern ret;

template<typename Writeable, std::enable_if_t<
        std::is_same<std::decay_t<Writeable>, ref_string_writer>{}
        || std::is_same<std::decay_t<Writeable>, rvalue_string_writer>{}, int> = 0>
auto
operator<<(Writeable &&w, ret_t) {
    return std::forward<typename Writeable::ref_type>(w.str());
}


template<typename Writer, typename Enum,
         std::enable_if_t<std::is_enum<Enum>{}, int> = 0>
void
write(Writer &w, Enum value) {
    auto map = enum_names<Enum>{}();
    write(w, map.first);

    const char *name = nullptr;
    for (auto &pair : map.second) {
        if (pair.first == value) {
            name = pair.second;
            break;
        }
    }
    if (name) {
        write(w, name);
    } else {
        write(w, "<");
        write(w, static_cast<std::underlying_type_t<Enum>>(value));
        write(w, ">");
    }
}


template<typename OutStream>
class stream_writer final: public writer {
public:
    stream_writer(OutStream &s)
        : m_stream(&s) {
    }

protected:
    virtual void v_write(const char *seq, std::size_t n) override {
        m_stream->put(seq, n);
    }

private:
    OutStream *m_stream;
};


template<std::size_t Index = 0, typename Writeable = void, typename Tuple = void,
         std::enable_if_t<Index < std::tuple_size<Tuple>{}, int> = 0>
void dispatch_write_tuple_element(Writeable &writer, const Tuple &args, std::size_t i) {
    if (Index == i) {
        dispatch_write(writer, std::get<Index>(args));
    } else {
        dispatch_write_tuple_element<Index+1>(writer, args, i);
    }
}


template<std::size_t Index = 0, typename Writeable = void, typename Tuple = void,
         std::enable_if_t<Index >= std::tuple_size<Tuple>{}, int> = 0>
void dispatch_write_tuple_element(Writeable &w, const Tuple &, std::size_t) {
    write(w, "??");
}


template<typename CharSequence, typename Writer, typename ArgTuple>
void
write_formatted(Writer &w, const CharSequence &fmt_str, const ArgTuple args) {
    class mutator final: public format_mod<Writer> {
    public:
        bitfield<fmt> new_flags {}, flag_mask {};
        unsigned new_width, new_precision;
        bool width_mask = false, precision_mask = false;

    protected:
        virtual bitfield<fmt> v_flags() const noexcept override {
            return format_mod<Writer>::v_flags() & ~flag_mask | new_flags;
        }

        virtual unsigned v_width() const noexcept override {
            return width_mask ? new_width : format_mod<Writer>::v_width();
        }

        virtual unsigned v_precision() const noexcept override {
            return precision_mask ? new_precision : format_mod<Writer>::v_precision();
        }
    };

    std::size_t next_arg = 0;
    const char *begin = &fmt_str[0], *start = begin, *it = start;
    for (;;) {
        if ((!*it || *it == '{') && start != it) {
            w.write(start, it-start);
        }
        if (*it == '{') {
            ++it;
            if (!*it || *it == '{') {
                write(w, "{");
            } else {
                mutator mut;
                mut.bind(w);

                if (*it >= '0' && *it <= '9') {
                    next_arg = static_cast<std::size_t>(*it - '0');
                    while (++it, *it >= '0' && *it <= '9') {
                        next_arg = next_arg * 10 + static_cast<std::size_t>(*it - '0');
                    }
                }

                if (*it == '<' || *it == '|' || *it == '>') {
                    mut.flag_mask |= fmt::justify_mask;
                    mut.new_flags = mut.new_flags & ~fmt::justify_mask
                            | (*it == '<' ? fmt::left : *it == '|' ? fmt::center : fmt::right);
                    if (*it >= '0' && *it <= '9') {
                        mut.width_mask = true;
                        mut.new_width = *it - '0';
                        while (++it, *it >= '0' && *it <= '9') {
                            mut.new_width = mut.new_width * 10 + *it - '0';
                        }
                    }
                }

                bitfield<fmt> base {}, float_rep {};
                bool set_base = false, set_float = false;
                for (;;) {
                    bool end = false;
                    switch (*it) {
                        case 'x': set_base = true; base = fmt::hex; break;
                        case 'X': set_base = true; base = fmt::hex | fmt::uppercase; break;
                        case 'o': set_base = true; base = fmt::oct; break;
                        case 'd': set_base = true; base = {}; break;
                        case 'g': set_float = true; float_rep = {}; break;
                        case 'f': set_float = true; float_rep = fmt::fixed; break;
                        case 'e': set_float = true; float_rep = fmt::sci; break;
                        case 'E': set_float = true; float_rep = fmt::sci | fmt::uppercase; break;
                        default: end = true;
                    }
                    if (end) break;
                    ++it;
                }
                if (set_base) {
                    mut.flag_mask |= fmt::base_mask;
                    mut.new_flags = mut.new_flags & ~fmt::base_mask | base;
                }
                if (set_float) {
                    mut.flag_mask |= fmt::float_mask;
                    mut.new_flags = mut.new_flags & ~fmt::float_mask | float_rep;
                }

                if (*it == '}') {
                    dispatch_write_tuple_element<0>(mut, args, next_arg++);
                } else {
                    write(w, "??");
                }
                ++it;
                start = it;
            }
        } else if (*it) {
            ++it;
        } else {
            break;
        }
    }
}


template<typename ...Params>
auto
format(const std::string &fmt, const Params &...arg_list) {
    auto args = std::make_tuple(arg_list...);
    return make_formatter([=](auto &w) {
        write_formatted(w, fmt, args);
    });
}


template<typename CharSequence, typename ...Params>
std::string
sprintf(const CharSequence fmt, Params &&...args) {
    string_writer sw;
    write_formatted(sw, fmt, std::make_tuple(args...));
    return sw.str();
}


namespace ops {

template<typename T>
auto
operator<<(std::string &str, T &&chain) {
    return ref_string_writer(str) << std::forward<T>(chain);
}

template<typename T>
auto
operator<<(std::string &&str, T &&chain) {
    return rvalue_string_writer(std::move(str)) << std::forward<T>(chain);
}

} // namespace ops


} // namespace sio
