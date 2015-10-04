#include "writer.hh"
#include <string>


namespace fio {


class string_writer: public writer {
public:
    string_writer(std::string &str)
        : m_string(&str) {
    }

    ~string_writer() noexcept {
        try { flush(); } catch(...) {}
    }

    void write(const char *seq, std::size_t n) {
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

private:
    static constexpr std::size_t sz = 100;
    std::string *m_string;
    char scratch[sz];
    std::size_t spos = 0;
};


namespace ops {

template<typename T>
auto operator<<(std::string &str, T &&chain) {
    return string_writer(str) << std::forward<T>(chain);
}

} // namespace ops

} // namespace fio
