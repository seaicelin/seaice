#include "zlib_stream.h"
#include "macro.h"

namespace seaice {

ZlibStream::ptr ZlibStream::CreateGzip(bool encode, uint32_t buff_size) {
    return Create(encode, buff_size, GZIP);
}

ZlibStream::ptr ZlibStream::CreateZlib(bool encode, uint32_t buff_size) {
    return Create(encode, buff_size, ZLIB);
}

ZlibStream::ptr ZlibStream::CreateDeflate(bool encode, uint32_t buff_size) {
    return Create(encode, buff_size, DEFLATE);
}

ZlibStream::ptr ZlibStream::Create(bool encode, uint32_t buff_size
                                , Type type, int level
                                , int window_bits, int memlevel
                                , Strategy stragey) {
    ZlibStream::ptr rt(new ZlibStream(encode, buff_size));
    if(rt->init(type, level, window_bits, memlevel, stragey) == Z_OK) {
        return rt;
    }
    return nullptr;
}

ZlibStream::ZlibStream(bool encode, uint32_t buff_size)
    : m_encode(encode)
    , m_buffSize(buff_size)
    , m_free(true) {
}

ZlibStream::~ZlibStream() {
    if(m_free) {
        for(auto& i : m_buffs) {
            free(i.iov_base);
        }
    }
    if(m_encode) {
        deflateEnd(&m_zs);
    } else {
        inflateEnd(&m_zs);
    }
}

int ZlibStream::read(void* buffer, size_t length) {
    throw std::logic_error("ZlibStream::read is invalid");
}
int ZlibStream::read(ByteArray::ptr ba, size_t length) {
    throw std::logic_error("ZlibStream::read is invalid");
}
int ZlibStream::write(const void* buffer, size_t length) {
    iovec iov;
    iov.iov_base = (void*)buffer;
    iov.iov_len = length;
    if(m_encode) {
        return encode(&iov, 1, false);
    } else {
        return decode(&iov, 1, false);
    }
}

int ZlibStream::write(ByteArray::ptr ba, size_t length) {
    std::vector<iovec> buffers;
    ba->getReadBuffers(buffers, length);
    if(m_encode) {
        return encode(&buffers[0], buffers.size(), false);
    } else {
        return decode(&buffers[0], buffers.size(), false);
    }
}

void ZlibStream::close() {
    flush();
}

int ZlibStream::init(Type type, int level, int window_bits,
        int memlevel, Strategy stragey) {
    SEAICE_ASSERT((level >= 0 && level <= 9) || (level == DEFAULT_COMPRESSION));
    SEAICE_ASSERT((window_bits >= 8 && window_bits <= 15));
    SEAICE_ASSERT((memlevel >= 0 && memlevel <= 9));

    memset(&m_zs, 0, sizeof(m_zs));

    m_zs.zalloc = Z_NULL;
    m_zs.zfree = Z_NULL;
    m_zs.opaque = Z_NULL;

    switch(type) {
        case DEFLATE:
            window_bits = -window_bits;
            break;
        case GZIP:
            window_bits += 16;
        break;
        case ZLIB:
            break;
        default:
            break;
    }
    if(m_encode) {
        return deflateInit2(&m_zs, level, Z_DEFLATED
                    , window_bits, memlevel, (int)stragey);
    } else {
        return inflateInit2(&m_zs, window_bits);
    }
}

int ZlibStream::encode(const iovec* v, const uint64_t& size, bool finish) {
    int ret = 0;
    int flush = 0;
    for(uint64_t i = 0; i < size; ++i) {
        m_zs.avail_in = v[i].iov_len;
        m_zs.next_in = (Bytef*)v[i].iov_base;
        flush = finish? (i == size - 1? Z_FINISH : Z_NO_FLUSH) : Z_NO_FLUSH;
        iovec* ivc = nullptr;
        do{
            if(!m_buffs.empty() && m_buffs.back().iov_len != m_buffSize) {
                ivc = &m_buffs.back();
            } else {
                iovec vc;
                vc.iov_base = malloc(m_buffSize);
                vc.iov_len = 0;
                m_buffs.push_back(vc);
                ivc = &m_buffs.back();
            }
            m_zs.avail_out = m_buffSize - ivc->iov_len;
            m_zs.next_out = (Bytef*)ivc->iov_base + ivc->iov_len;

            ret = deflate(&m_zs, flush);
            if(ret == Z_STREAM_ERROR) {
                return ret;
            }
            ivc->iov_len = m_buffSize - m_zs.avail_out;
        }while(m_zs.avail_out == 0);
    }
    if(flush == Z_FINISH) {
        deflateEnd(&m_zs);
    }
    return Z_OK;
}

int ZlibStream::decode(const iovec* v, const uint64_t& size, bool finish) {
    int ret = 0;
    int flush = 0;
    for(uint64_t i = 0; i < size; i++) {
        m_zs.avail_in = v[i].iov_len;
        m_zs.next_in = (Bytef*)v[i].iov_base;
        flush = finish? (i == size - 1 ? Z_FINISH : Z_NO_FLUSH) : Z_NO_FLUSH;
        iovec* ivc = nullptr;
        do{
            if(!m_buffs.empty() && m_buffs.back().iov_len != m_buffSize) {
                ivc = &m_buffs.back();
            } else {
                iovec vc;
                vc.iov_base = malloc(m_buffSize);
                vc.iov_len = 0;
                m_buffs.push_back(vc);
                ivc = &m_buffs.back();
            }

            m_zs.avail_out = m_buffSize - ivc->iov_len;
            m_zs.next_out = (Bytef*)ivc->iov_base + ivc->iov_len;

            ret = inflate(&m_zs, flush);
            if(ret == Z_STREAM_ERROR) {
                return ret;
            }
            ivc->iov_len = m_buffSize - m_zs.avail_out;
        }while(m_zs.avail_out == 0);
    }
    if(flush == Z_FINISH) {
        inflateEnd(&m_zs);
    }
    return Z_OK;
}

int ZlibStream::flush() {
    iovec iov;
    iov.iov_base = nullptr;
    iov.iov_len = 0;
    if(m_encode) {
        return encode(&iov, 1, true);
    } else {
        return decode(&iov, 1, true);
    }
}

seaice::ByteArray::ptr ZlibStream::getByteArray() {
    seaice::ByteArray::ptr ba(new ByteArray);
    for(auto& i : m_buffs) {
        ba->write((const char*)i.iov_base, i.iov_len);
    }
    ba->setPos(0);
    return ba;
}

std::string ZlibStream::getResult() const {
    std::string rt;
    for(auto& i : m_buffs) {
        rt.append((const char*)i.iov_base, i.iov_len);
    }
    return rt;
}

}