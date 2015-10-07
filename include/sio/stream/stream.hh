#pragma once

#include <cstdint>
#include <vector>
#include <limits>
#include "../enum.hh"
#include <string>


namespace std {

    template<typename, typename>
    class basic_streambuf;

}


namespace sio {


enum class seek {
    set,
    cur,
    end
};

template<>
struct enum_names<seek> {
    enum_name_list<seek> operator()() const {
        return { "sio::seek::", {
            { seek::set, "set" }, { seek::cur, "cur" }, { seek::end, "end" }
        } };
    }
};


using stream_pos = std::uint_least64_t;
using stream_off = std::make_signed_t<stream_pos>;


class stream {
public:
    virtual inline ~stream() = 0;
};

stream::~stream() {}


class in_stream: public virtual stream {
protected:
    virtual std::size_t v_get(void *out, std::size_t bytes) = 0;

public:
    std::size_t get(void *out, std::size_t bytes) {
        return v_get(out, bytes);
    }
};


class out_stream: public virtual stream {
protected:
    virtual std::size_t v_put(const void *in, std::size_t bytes) = 0;

    virtual void v_flush() {}

public:
    std::size_t put(const void *in, std::size_t bytes) {
        return v_put(in, bytes);
    }

    void flush() {
        v_flush();
    }
};


class read_stream: public virtual in_stream {
protected:
    virtual stream_pos v_seek_get(stream_off offset, sio::seek rel) = 0;

    virtual stream_pos v_tell_get() const = 0;

public:
    stream_pos seek_get(stream_off offset, sio::seek rel) {
        return v_seek_get(offset, rel);
    }

    stream_pos seek_get(stream_pos new_pos) {
        return v_seek_get(static_cast<stream_off>(new_pos), sio::seek::set);
    }

    stream_pos seek(stream_off offset, sio::seek rel) {
        return v_seek_get(offset, rel);
    }

    stream_pos seek(stream_pos new_pos) {
        return v_seek_get(static_cast<stream_off>(new_pos), sio::seek::set);
    }

    stream_pos tell_get() const {
        return v_tell_get();
    }

    stream_pos tell() const {
        return v_tell_get();
    }
};


class write_stream: public virtual out_stream {
protected:
    virtual stream_pos v_seek_put(stream_off offset, sio::seek rel) = 0;

    virtual stream_pos v_tell_put() const = 0;

public:
    stream_pos seek_put(stream_off offset, sio::seek rel) {
        return v_seek_put(offset, rel);
    }

    stream_pos seek_put(stream_pos new_pos) {
        return v_seek_put(static_cast<stream_off>(new_pos), sio::seek::set);
    }

    stream_pos seek(stream_off offset, sio::seek rel) {
        return v_seek_put(offset, rel);
    }

    stream_pos seek(stream_pos new_pos) {
        return v_seek_put(static_cast<stream_off>(new_pos), sio::seek::set);
    }

    stream_pos tell_put() const {
        return v_tell_put();
    }

    stream_pos tell() const {
        return v_tell_put();
    }
};


class duplex_stream: public virtual in_stream, public virtual out_stream {};

class rw_stream: public virtual read_stream, public virtual write_stream,
    public duplex_stream {};


template<typename Stream>
using is_in_stream = std::is_base_of<in_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_out_stream = std::is_base_of<out_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_read_stream = std::is_base_of<read_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_write_stream = std::is_base_of<write_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_duplex_stream = std::is_base_of<duplex_stream, std::decay_t<Stream>>;

template<typename Stream>
using is_rw_stream = std::is_base_of<rw_stream, std::decay_t<Stream>>;


class memory_stream final: public rw_stream {
protected:
    virtual std::size_t v_get(void *out, std::size_t bytes) override {
        if (m_data.empty()) return 0;

        bytes = std::max(bytes, m_data.size() - static_cast<std::size_t>(m_pos));
        auto start = &m_data[0] + m_pos;
        std::copy(start, start + bytes,  static_cast<unsigned char*>(out));
        return bytes;
    }

    virtual std::size_t v_put(const void *in, std::size_t bytes) override {
        if (static_cast<stream_pos>(m_pos) + bytes > max_pos) {
            bytes = static_cast<std::size_t>(max_pos) - m_pos;
        }
        if (m_pos + bytes > m_data.size()) {
            m_data.resize(m_pos + bytes);
        }
        auto byte_in = static_cast<const unsigned char*>(in);
        std::copy(byte_in, byte_in + bytes, &m_data[0] + m_pos);
        return bytes;
    }

    virtual stream_pos v_seek_get(stream_off offset, sio::seek rel) override {
        return seek_both(offset, rel);
    }

    virtual stream_pos v_tell_get() const override {
        return tell_both();
    }

    virtual stream_pos v_seek_put(stream_off offset, sio::seek rel) override {
        return seek_both(offset, rel);
    }

    virtual stream_pos v_tell_put() const override {
        return tell_both();
    }

public:
    using write_stream::seek;
    using write_stream::tell;

private:
    stream_pos seek_both(stream_off offset, sio::seek rel) {
        stream_off new_pos;
        switch (rel) {
            case sio::seek::set: new_pos = offset; break;
            case sio::seek::cur: new_pos = static_cast<stream_off>(m_pos) + offset; break;
            case sio::seek::end: new_pos = static_cast<stream_off>(m_data.size()) - offset; break;
        }
        if (new_pos < 0) new_pos = 0;
        if (new_pos > static_cast<stream_off>(max_pos)) new_pos = static_cast<stream_off>(max_pos);
        return m_pos;
    }

    stream_pos tell_both() const {
        return m_pos;
    }

    std::size_t m_pos = 0;
    std::vector<unsigned char> m_data;

    enum : stream_pos { max_pos = std::numeric_limits<std::size_t>::max() };
};


enum class open_mode {
    overwrite, append, truncate
};

template<>
struct enum_names<open_mode> {
    using e = open_mode;
    enum_name_list<e> operator()() const {
        return { "sio::write_mode::", {
            { e::overwrite, "overwrite" }, { e::append, "append" }, { e::truncate, "truncate" }
        } };
    }
};


class file_stream {
public:
    using streambuf_type = std::basic_streambuf<char, std::char_traits<char>>;

    virtual ~file_stream() = 0;

    file_stream(const file_stream&) = delete;
    file_stream(file_stream&&) = delete;
    file_stream &operator=(const file_stream&) = delete;
    file_stream &operator=(file_stream&&) = delete;

    explicit file_stream(streambuf_type &buf)
        : m_streambuf(&buf), m_owns_streambuf(false) {
    }

protected:
    enum { allow_get = 1, allow_put = 2 };

    file_stream(const std::string &fname, int perm,
            open_mode mode = open_mode::truncate);

    const streambuf_type &streambuf() const { return *m_streambuf; }
    streambuf_type &streambuf() { return *m_streambuf; }

private:
    std::basic_streambuf<char, std::char_traits<char>> *m_streambuf;
    bool m_owns_streambuf;
};


class file_in_stream: public virtual file_stream, public virtual in_stream {
public:
    explicit file_in_stream(streambuf_type &buf)
        : file_stream(buf) {
    }

    explicit file_in_stream(const std::string &fname)
        : file_stream(fname, allow_get) {
    }

protected:
    virtual std::size_t v_get(void *out, std::size_t bytes) final override;
};


class file_out_stream: public virtual file_stream, public virtual out_stream {
public:
    explicit file_out_stream(streambuf_type &buf)
        : file_stream(buf) {
    }

    explicit file_out_stream(const std::string &fname, open_mode mode = open_mode::truncate)
        : file_stream(fname, allow_put, mode) {
    }

protected:
    virtual std::size_t v_put(const void *in, std::size_t bytes) final override;

    virtual void v_flush() final override;
};



class file_duplex_stream: public virtual file_in_stream, public virtual file_out_stream {
public:
    explicit file_duplex_stream(streambuf_type &buf)
        : file_stream(buf), file_in_stream(buf), file_out_stream(buf) {
    }

    explicit file_duplex_stream(const std::string &fname, open_mode mode = open_mode::truncate)
        : file_stream(fname, allow_get | allow_put, mode),
          file_in_stream(streambuf()), file_out_stream(streambuf()) {
    }
};


class file_read_stream: public virtual file_in_stream, public virtual read_stream {
public:
    explicit file_read_stream(streambuf_type &buf)
        : file_stream(buf), file_in_stream(buf) {
    }

    explicit file_read_stream(const std::string &fname)
        : file_stream(fname, allow_get), file_in_stream(streambuf())  {
    }

protected:
    virtual stream_pos v_seek_get(stream_off offset, sio::seek rel) final override;

    virtual stream_pos v_tell_get() const final override;
};


class file_write_stream: public virtual file_out_stream, public virtual write_stream {
public:
    explicit file_write_stream(streambuf_type &buf)
        : file_stream(buf), file_out_stream(buf) {
    }

    explicit file_write_stream(const std::string &fname, open_mode mode = open_mode::truncate)
        : file_stream(fname, allow_put, mode), file_out_stream(streambuf())  {
    }

protected:
    virtual stream_pos v_seek_put(stream_off offset, sio::seek rel) final override;

    virtual stream_pos v_tell_put() const final override;
};


class file_rw_stream final: public file_read_stream, public file_write_stream,
        public file_duplex_stream, public rw_stream {
public:
    explicit file_rw_stream(streambuf_type &buf)
        : file_stream(buf), file_in_stream(buf), file_out_stream(buf),
          file_read_stream(buf), file_write_stream(buf), file_duplex_stream(buf) {
    }

    explicit file_rw_stream(const std::string &fname, open_mode mode = open_mode::truncate)
        : file_stream(fname, allow_get | allow_put, mode), file_in_stream(streambuf()),
          file_out_stream(streambuf()), file_read_stream(streambuf()),
          file_write_stream(streambuf()), file_duplex_stream(streambuf()) {
    }

    stream_pos seek(stream_off offset, sio::seek rel);

    stream_pos seek(stream_pos new_pos) {
        return seek(static_cast<stream_off>(new_pos), sio::seek::set);
    }
};


extern file_out_stream stdout_stream;
extern file_out_stream stderr_stream;
extern file_out_stream buffered_stderr_stream;
extern file_in_stream stdin_stream;


} // namespace sio