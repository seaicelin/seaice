#include "test.h"
#include "../seaice/bytearray.h"

static seaice::Logger::ptr logger = SEAICE_LOGGER("seaice");

using namespace std;
using namespace seaice;

void test() {


    //int64_t a = 2;
    //ba->writeInt64(a);
    //ba->setPos(0);
    //int64_t b = ba->readInt64();
    //SEAICE_LOG_DEBUG(logger) << "ret = " << (unsigned)b;
        /*
        SEAICE_LOG_DEBUG(logger) << "i = " << i << \
            " v = " << v << " vec[i] = " << vec[i]; \
                SEAICE_LOG_DEBUG(logger) << "ba = " << ba->toHexString(); \
    SEAICE_LOG_DEBUG(logger) << "ba2 = " << ba2->toHexString(); \*/

#define XX(type, len, write_fun, read_fun, base_len) { \
    std::vector<type> vec; \
    for(int i = 0; i < len; i++) { \
        vec.push_back(rand()); \
    } \
    seaice::ByteArray::ptr ba(new seaice::ByteArray(base_len)); \
    for(auto& i : vec) { \
        ba->write_fun(i); \
    } \
    ba->setPos(0); \
    for(size_t i = 0; i < vec.size(); i++) { \
        type v = ba->read_fun(); \
        SEAICE_ASSERT(v == vec[i]); \
    } \
    SEAICE_ASSERT(ba->getReadSize() == 0); \
    SEAICE_LOG_DEBUG(logger) << "get read size = " << ba->getReadSize() \
        << " get base size = " << ba->getBaseSize() << " get pos = " << \
        ba->getPos() << " get size = " << ba->getSize(); \
    }

    XX(int8_t, 1000, writeFint8, readFint8, 1);
    XX(uint8_t, 1000, writeFuint8, readFuint8, 1);

    XX(int16_t, 1000, writeFint16, readFint16, 1);
    XX(uint16_t, 1000, writeFuint16, readFuint16, 1);

    XX(int32_t, 1000, writeFint32, readFint32, 1);
    XX(uint32_t, 1000, writeFuint32, readFuint32, 1);

    XX(int64_t, 1000, writeFint64, readFint64, 1);
    XX(uint64_t, 1000, writeFuint64, readFuint64, 1);

    XX(int32_t, 1000, writeInt32, readInt32, 1);
    XX(uint32_t, 1000, writeUint32, readUint32, 1);

    XX(int64_t, 1000, writeInt64, readInt64, 1);
    XX(uint64_t, 10000, writeUint64, readUint64, 1);
#undef XX

#define XX(type, len, write_fun, read_fun, base_len) { \
    std::vector<type> vec; \
    for(int i = 0; i < len; i++) { \
        vec.push_back(rand()); \
    } \
    seaice::ByteArray::ptr ba(new seaice::ByteArray(base_len)); \
    for(auto& i : vec) { \
        ba->write_fun(i); \
    } \
    ba->setPos(0); \
    for(size_t i = 0; i < vec.size(); i++) { \
        type v = ba->read_fun(); \
        SEAICE_ASSERT(v == vec[i]); \
    } \
    SEAICE_ASSERT(ba->getReadSize() == 0); \
    SEAICE_LOG_DEBUG(logger) << "get read size = " << ba->getReadSize() \
        << " get base size = " << ba->getBaseSize() << " get pos = " << \
        ba->getPos() << " get size = " << ba->getSize(); \
    ba->setPos(0); \
    SEAICE_ASSERT(ba->writeToFile("/tmp/" #type "_" #len "_" #read_fun ".dat")); \
    seaice::ByteArray::ptr ba2(new seaice::ByteArray(base_len * 2)); \
    SEAICE_ASSERT(ba2->readFromFile("/tmp/" #type "_" #len "_" #read_fun ".dat")); \
    ba2->setPos(0); \
    SEAICE_ASSERT(ba->toString() == ba2->toString()); \
    SEAICE_ASSERT(ba->toHexString() == ba2->toHexString()); \
    SEAICE_ASSERT(ba->getPos() == 0); \
    SEAICE_ASSERT(ba2->getPos() == 0); \
}

    XX(int8_t, 10, writeFint8, readFint8, 1);
    XX(uint8_t, 10, writeFuint8, readFuint8, 1);

    XX(int16_t, 1000, writeFint16, readFint16, 1);
    XX(uint16_t, 1000, writeFuint16, readFuint16, 1);

    XX(int32_t, 1000, writeFint32, readFint32, 1);
    XX(uint32_t, 1000, writeFuint32, readFuint32, 1);

    XX(int64_t, 1000, writeFint64, readFint64, 1);
    XX(uint64_t, 1000, writeFuint64, readFuint64, 1);

    XX(int32_t, 1000, writeInt32, readInt32, 1);
    XX(uint32_t, 1000, writeUint32, readUint32, 1);

    XX(int64_t, 1000, writeInt64, readInt64, 1);
    XX(uint64_t, 10000, writeUint64, readUint64, 1);
#undef XX


}

int main() {
    test();
    return 0;
}