#include "stream.hh"
#include <string>


namespace std {

    template<typename, typename>
    class basic_streambuf;

}


namespace sio {


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
