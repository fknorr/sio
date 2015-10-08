#include "stream.hh"
#include <vector>
#include <limits>


namespace sio {


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


} // namespace sio
