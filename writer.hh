#pragma once

#include <type_traits>
#include <utility>
#include <string>
#include <memory>
#include <locale>


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
            std::unique_ptr<std::ios_base> ios_base;
            const std::locale *locale;
            const std::num_put<char> *num_put;
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
    };

    ios_cache &ios() const {
        return m_ios;
    }

    writer() {}
    writer(const writer &) {}
    writer(writer &&rhs) = default;

    writer &operator=(const writer&) = delete;
    writer &operator=(writer &&) = default;

    const std::locale &locale() const {
        return std::locale::classic();
    }

    void flush() {}

private:
    mutable ios_cache m_ios;
};


template<typename Facet>
const Facet &
writer::ios_cache::cached_facet(Facet *&ptr, const std::locale &locale) {
    if (!ptr) {
        ptr = &std::use_facet<Facet>(locale);
    }
    return *ptr;
}


template<>
inline const std::num_put<char> &
writer::ios_cache::locale_facet<std::num_put<char>>(const std::locale &locale) {
    auto &r = refs(locale);
    return cached_facet(r.num_put, *r.locale);
}



template<typename T>
using is_writer = std::is_base_of<writer, std::decay_t<T>>;


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
auto operator<<(const WriterMod &w, const Next &next) {
    write(w, next);
    return w.base();
}


template<typename Writer, typename Next,
        std::enable_if_t<is_writer<Writer>{} && !is_free_writer_mod<Writer>{}, int> = 0>
auto operator<<(Writer &&w, const Next &next) {
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
auto operator<<(WriterModHook &&w, end_t) {
    return w.parent();
}


class nl_t {} extern nl;


template<typename Writer>
auto write(Writer &w, nl_t) {
    /*switch (writer.line_ending()) {
        case line_ending::cr: writer.write("\r", 1); break;
        case line_ending::lf: */w.write("\n", 1);/* break;
        case line_ending::crlf: writer.write("\r\n", 2); break;
    }*/
}


class string_writer: public writer {
public:
    string_writer(std::string &str)
        : m_string(&str) {
    }

    void write(const char *seq, std::size_t n) {
        if (spos + n > sz) {
            flush();
        }
        if (n > sz) {
            m_string->append(seq, seq+n);
        } else {
            std::copy(seq, seq+n, scratch);
            spos = n;
        }
    }

    void flush() {
        if (spos) {
            m_string->append(scratch, scratch+spos);
            spos = 0;
        }
    }

private:
    static constexpr std::size_t sz = 100;
    std::string *m_string;
    char scratch[sz];
    std::size_t spos = 0;
};


template<typename Writer>
void write(Writer &w, int v);

template<typename T>
auto operator<<(std::string &str, T &&chain) {
    return string_writer(str) << std::forward<T>(chain);
}
