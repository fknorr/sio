#pragma once

#include <type_traits>
#include <utility>
#include <memory>


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


namespace fio {


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


enum class line_ending {
    cr,
    lf,
    crlf
};


class writer {
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

    ios_cache &ios() const {
        return m_ios;
    }

    writer() noexcept {}
    writer(const writer &) noexcept {}
    writer(writer &&rhs) = default;

    writer &operator=(const writer&) = delete;
    writer &operator=(writer &&) = default;

    const std::locale &locale() const;

    fio::line_ending line_ending() const noexcept {
        return line_ending::lf;
    }

private:
    mutable ios_cache m_ios;
};


template<typename T>
using is_writer = std::is_base_of<writer, std::decay_t<T>>;


class buffered {};

template<typename T>
using is_buffered = std::is_base_of<buffered, std::decay_t<T>>;

template<typename T>
using is_buffered_writer = std::integral_constant<bool, is_writer<T>{} && is_buffered<T>{}>;


class writer_mod {};

template<typename T>
using is_writer_mod = std::is_base_of<writer_mod, std::decay_t<T>>;

class writer_mod_hook {};

template<typename T>
using is_writer_mod_hook = std::is_base_of<writer_mod_hook, std::decay_t<T>>;

template<typename T>
using is_free_writer_mod = std::integral_constant<bool,
        is_writer_mod<std::decay_t<T>>{} && !is_writer_mod_hook<std::decay_t<T>>{}>;



template<typename Writer, typename WriterMod,
        std::enable_if_t<is_writer<Writer>{} && is_writer_mod<WriterMod>{}, int> = 0>
auto operator<<(Writer &w, const WriterMod &mod) {
    return mod.bind(w);
}


template<typename WriterMod, typename Next,
        std::enable_if_t<is_free_writer_mod<WriterMod>{}, int> = 0>
decltype(auto) operator<<(const WriterMod &w, const Next &next) {
    write(w, next);
    return w.base();
}


template<typename Writer, typename Next,
        std::enable_if_t<is_writer<Writer>{} && !is_free_writer_mod<Writer>{}, int> = 0>
decltype(auto) operator<<(Writer &&w, const Next &next) {
    write(w, next);
    return std::forward<Writer>(w);
}


template<typename Writer, typename Formatter,
        std::enable_if_t<is_formatter<Formatter>{}, int> = 0>
void write(Writer &w, const Formatter &fmt) {
    fmt(w);
}


template<typename WriterMod, std::enable_if_t<is_free_writer_mod<WriterMod>{}, int> = 0>
auto begin(WriterMod &&mod) {
    class hook_impl: public writer_mod_hook, public std::decay_t<WriterMod> {
        public:
            hook_impl(WriterMod &&mod)
                : WriterMod(std::forward<WriterMod>(mod)) {
            }
    };
    return hook_impl(std::forward<WriterMod>(mod));
}


class end_t {} extern end;

template<typename WriterModHook, std::enable_if_t<is_writer_mod_hook<WriterModHook>{}, int> = 0>
decltype(auto) operator<<(WriterModHook &&w, end_t) {
    return w.parent();
}


class nl_t {} extern nl;

template<typename Writer>
void write(Writer &w, nl_t) {
    switch (w.line_ending()) {
        case line_ending::cr: write(w, "\r"); break;
        case line_ending::lf: write(w, "\n"); break;
        case line_ending::crlf: write(w, "\r\n"); break;
    }
}


class flush_t {} extern flush;

template<typename BufferedWriter,
        std::enable_if_t<is_buffered_writer<BufferedWriter>{}, int> = 0>
void write(BufferedWriter &w, flush_t) {
    w.flush();
}



template<typename Writer, std::size_t N>
void write(Writer &w, const char (&literal)[N]) {
    w.write(literal, N-1);
}


template<typename Writer>
auto write(Writer &w, const std::string &str) {
    w.write(str.c_str(), str.length());
}



template<typename Writer>
void write(Writer &w, int v);


} // namespace fio
