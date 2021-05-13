#ifndef __SEAICE_ZLIB_STREAM_H_
#define __SEAICE_ZLIB_STREAM_H_

#include "stream.h"
#include <zlib.h>
#include <vector>
#include <sys/uio.h>

/*
理解和使用zlib库
https://blog.csdn.net/querw/article/details/51569274
https://www.cnblogs.com/en-heng/p/4992916.html

//typedef struct z_stream_s {
    //z_const Bytef *next_in;     // next input byte 
    //uInt     avail_in;  // number of bytes available at next_in 
    //uLong    total_clearin;  // total number of input bytes read so far 

    //Bytef    *next_out; // next output byte should be put there 
    //uInt     avail_out; // remaining free space at next_out 
    //uLong    total_out; // total number of bytes output so far 

    //z_const char *msg;  // last error message, NULL if no error
    //struct internal_state FAR *state; / not visible by applications 

    //alloc_func zalloc;  // used to allocate the internal state 
    //free_func  zfree;   // used to free the internal state 
    //voidpf     opaque;  // private data object passed to zalloc and zfree 

    //int     data_type;  // best guess about the data type: binary or text 
    //uLong   adler;      // adler32 value of the uncompressed data 
    //uLong   reserved;   // reserved for future use 
//} z_stream;

/**
由输入数据 xxxx_in 和输出数据 xxxx_out 组成,原始数据从输入端流入,
变为压缩数据从输出端流出(解压缩反过来).
编程的时候,用户不停的"喂"数据到 next_in 并指定它的长度 avail_in 
调用压缩函数,然后从 next_out 得到压缩后的数据,长度是 avail_out.
这就是整个 zlib 库的接口的设计思路.
msg: 错误信息
zalloc / zfree / opaque: 类似于 C++ STL 中的 allocator 的作用,
如果要定制内存管理可以自己编写内存分配回收函数.
adler: adler32 / CRC32 校验和.
**/

namespace seaice{

class ZlibStream : public Stream
{
public:
    typedef std::shared_ptr<ZlibStream> ptr;

    //deflate算法, zlib 格式, gzip 格式
    enum Type
    {
        ZLIB,
        DEFLATE,
        GZIP
    };

    //内部压缩算法的编码策略,如果没有特殊要求,设置为 Z_DEFAULT_STRATEGY
    enum Strategy {
        DEFAULT = Z_DEFAULT_STRATEGY,
        FILTERED = Z_FILTERED,
        HUFFMAN = Z_HUFFMAN_ONLY,
        FIXED = Z_FIXED,
        RLE = Z_RLE
    };

    //level: 压缩级别 0 ~ 9. 0 表示不压缩, 1 表示速度最快, 
    //9 表示压缩比最高. Z_DEFAULT_COMPRESSION (-1) 表示使用默认设置.
    enum CompressLevel {
        NO_COMRESSION = Z_NO_COMPRESSION,
        BEST_SPEED = Z_BEST_SPEED,
        BEST_COMPRESSION = Z_BEST_COMPRESSION,
        DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION
    };

    //encode 表示编码还是解码
    static ZlibStream::ptr CreateGzip(bool encode, uint32_t buff_size = 4096);
    static ZlibStream::ptr CreateZlib(bool encode, uint32_t buff_size = 4096);
    static ZlibStream::ptr CreateDeflate(bool encode, uint32_t buff_size = 4096);
    /*
    不同包装格式的数据输出由 windowBits 这个参数控制:
        8 ~ 15: 输出 zlib 数据头/尾, deflateInit() 中这个参数值固定为 15, 就是 zconf.h 中定义的 MAX_WBITS 的值.
        -8 ~ -15: 输出原始的压缩数据不含任何数据头/尾. 如果没有特殊要求,使用 -15 就可以,表示内部使用 32K 的 LZ77 滑动窗口.
        24 ~ 31: 输出 gzip 格式的数据,默认提供一个所有设置都清零的 gzip 数据头,如果要自定义这个数据头,
            可以在初始化之后, deflate() 之前调用 deflateSetHeader().
    level: 压缩级别 0 ~ 9. 0 表示不压缩, 1 表示速度最快, 9 表示压缩比最高. Z_DEFAULT_COMPRESSION (-1) 表示使用默认设置.
    method: Z_DEFLATED(8) 只是唯一支持的压缩算法.
    memLevel: 控制 libzlib 内部使用内存的大小, 1 ~ 9 数字越小内存用量也越小,花费时间越多.默认值是8.
    strategy: 内部压缩算法的编码策略,如果没有特殊要求,设置为 Z_DEFAULT_STRATEGY 就可以了
        (如果你有特殊要求,那你自然知道其余选项 Z_FILTERED / Z_HUFFMAN_ONLY / Z_RLE / Z_FIXED 是什么意思).
    */
    static ZlibStream::ptr Create(bool encode, uint32_t buff_size = 4096
                                , Type type = DEFLATE, int level = DEFAULT_COMPRESSION
                                , int window_bits = 15, int memlevel = 8
                                , Strategy stragey = DEFAULT);

    ZlibStream(bool encode, uint32_t buff_size = 4096);
    ~ZlibStream();

    virtual int read(void* buffer, size_t length) override;
    virtual int read(ByteArray::ptr ba, size_t length) override;
    virtual int write(const void* buffer, size_t length) override;
    virtual int write(ByteArray::ptr ba, size_t length) override;
    virtual void close() override;

    int flush();

    bool isFree() const {return m_free;}
    void setFree(bool v) {m_free = v;}
    bool isEncdoe() const {return m_encode;}
    void setEncode(bool v) {m_encode = v;}

    std::vector<iovec>& getBuffers() {return m_buffs;};
    std::string getResult() const;

private:
    int init(Type type = DEFLATE, int level = DEFAULT_COMPRESSION
            ,int window_bits = 15, int memlevel = 8, Strategy stragey = DEFAULT);
    int encode(const iovec* v, const uint64_t& size, bool finish);
    int decode(const iovec* v, const uint64_t& size, bool finish);

private:
    bool m_encode;
    uint32_t m_buffSize;
    bool m_free;
    z_stream m_zs;
    std::vector<iovec> m_buffs;
};

}
#endif