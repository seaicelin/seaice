#include "test.h"
#include "../seaice/zlib_stream.h"


void test_gzip() {
    std::cout<< "================gzip====================" << std::endl;
    std::string data;
    for(int i = 0; i < 100000; i++) {
        data.append("a" + std::to_string(i));
    }
    auto gzip_compress = seaice::ZlibStream::CreateGzip(true, 1);
    std::cout << "compress: " << gzip_compress->write(data.c_str(), data.size())
              << " length: " << gzip_compress->getBuffers().size()
              << std::endl;
    std::cout << "flush: " << gzip_compress->flush() << std::endl;
    auto compress_str = gzip_compress->getResult();

    auto gzip_uncompress = seaice::ZlibStream::CreateGzip(false, 1);
    std::cout <<"uncompress: " << gzip_uncompress->write(compress_str.c_str(), compress_str.size())
              <<" length: " << gzip_uncompress->getBuffers().size()
              << std::endl;
    std::cout << "flush: " << gzip_uncompress->flush() << std::endl;
    auto uncompress_str = gzip_uncompress->getResult();

    std::cout <<"test_gzip result = " << (data == uncompress_str)
        <<" origin_size: " << data.size()
        <<" uncompress.size: " << uncompress_str.size()
        <<" compress size: " << compress_str.size()
        << std::endl; 
}

void test_deflate() {
    std::cout<< "================deflate====================" << std::endl;
    std::string data;
    for(int i = 0; i < 100000; i++) {
        data.append("a" + std::to_string(i));
    }
    auto gzip_compress = seaice::ZlibStream::CreateDeflate(true, 1);
    std::cout << "compress: " << gzip_compress->write(data.c_str(), data.size())
              << " length: " << gzip_compress->getBuffers().size()
              << std::endl;
    std::cout << "flush: " << gzip_compress->flush() << std::endl;
    auto compress_str = gzip_compress->getResult();

    auto gzip_uncompress = seaice::ZlibStream::CreateDeflate(false, 1);
    std::cout <<"uncompress: " << gzip_uncompress->write(compress_str.c_str(), compress_str.size())
              <<" length: " << gzip_uncompress->getBuffers().size()
              << std::endl;
    std::cout << "flush: " << gzip_uncompress->flush() << std::endl;
    auto uncompress_str = gzip_uncompress->getResult();

    std::cout <<"test_gzip result = " << (data == uncompress_str)
        <<" origin_size: " << data.size()
        <<" uncompress.size: " << uncompress_str.size()
        <<" compress size: " << compress_str.size()
        << std::endl; 
}

void test_zlib() {
    std::cout<< "================zlib====================" << std::endl;
    std::string data;
    for(int i = 0; i < 100000; i++) {
        data.append("a" + std::to_string(i));
    }
    auto gzip_compress = seaice::ZlibStream::CreateZlib(true, 1);
    std::cout << "compress: " << gzip_compress->write(data.c_str(), data.size())
              << " length: " << gzip_compress->getBuffers().size()
              << std::endl;
    std::cout << "flush: " << gzip_compress->flush() << std::endl;
    auto compress_str = gzip_compress->getResult();

    auto gzip_uncompress = seaice::ZlibStream::CreateZlib(false, 1);
    std::cout <<"uncompress: " << gzip_uncompress->write(compress_str.c_str(), compress_str.size())
              <<" length: " << gzip_uncompress->getBuffers().size()
              << std::endl;
    std::cout << "flush: " << gzip_uncompress->flush() << std::endl;
    auto uncompress_str = gzip_uncompress->getResult();

    std::cout <<"test_gzip result = " << (data == uncompress_str)
        <<" origin_size: " << data.size()
        <<" uncompress.size: " << uncompress_str.size()
        <<" compress size: " << compress_str.size()
        << std::endl; 
}

int main(int argc, char const *argv[])
{
    srand(time(0));
    test_gzip();
    test_deflate();
    test_zlib();
    return 0;
}