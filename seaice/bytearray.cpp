#include "bytearray.h"
#include "endian.h"
#include "log.h"
#include "macro.h"
#include <fstream>
#include <string>
#include <string.h>
#include <iomanip>

namespace seaice{

static Logger::ptr logger = SEAICE_LOGGER("system");

ByteArray::Node::Node() 
    : size(0)
    , ptr(nullptr)
    , next(nullptr) {
}

ByteArray::Node::Node(size_t len) 
    : size(len)
    , ptr(new char[size])
    , next(nullptr){
}

ByteArray::Node::~Node() {
    if(ptr) {
        delete[] ptr;
    }
}
/*
ByteArray::ByteArray() 
    : m_baseSize(0)
    , m_pos(0)
    , m_cap(0)
    , m_size(0)
    , m_endian(SEAICE_BYTE_ORDER)
    , m_root(nullptr)
    , m_cur(nullptr){
}*/

ByteArray::ByteArray(size_t baseSize) 
    : m_baseSize(baseSize)
    , m_pos(0)
    , m_cap(baseSize)
    , m_size(0)
    , m_endian(SEAICE_BIG_ENDIAN)
    , m_root(new Node(baseSize))
    , m_cur(m_root){
}

ByteArray::~ByteArray() {
    m_cur = m_root;
    while(m_cur) {
        m_root = m_root->next;
        delete m_cur;
        m_cur = m_root;
    }
}

//编码
//https://blog.csdn.net/weixin_43708622/article/details/111397290
//https://izualzhy.cn/protobuf-encode-varint-and-zigzag

void ByteArray::writeFint8(int8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8(uint8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFint16(int16_t value) {
    if(m_endian != SEAICE_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) {
    if(m_endian != SEAICE_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint32(int32_t value) {
    if(m_endian != SEAICE_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) {
    if(m_endian != SEAICE_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64(int64_t value) {
    if(m_endian != SEAICE_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) {
    if(m_endian != SEAICE_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

static uint32_t EncodeZigzag32(const int32_t& value) {
    uint32_t ret = 0;
    if(value < 0) {
        ret = ((uint32_t)(-value)) * 2 - 1;
        //return ;
    } else {
        ret = value * 2;
        //return value * 2;
    }
    //SEAICE_LOG_DEBUG(logger) << "encode value = " <<
        //value << " to ret " << ret;
    return ret;
}

static uint64_t EncodeZigzag64(const int64_t& value) {
    if(value < 0) {
        return ((uint64_t)(-value)) * 2 - 1;
    } else {
        return value * 2;
    }
}

static int32_t DecodeZigzag32(const uint32_t& value) {
    int32_t ret = (value >> 1) ^ -(value & 1);
    //SEAICE_LOG_DEBUG(logger) << "decode value = " <<
        //value << " to ret " << ret;
    return ret;
    //return (value >> 1) ^ -(value & 1);
}

static int64_t DecodeZigzag64(const uint64_t& value) {
    return (value >> 1) ^ -(value & 1);
}

void ByteArray::writeInt32(int32_t value) {
    writeUint32(EncodeZigzag32(value));
}

void ByteArray::writeUint32(uint32_t value) {
    uint8_t tmp[5];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = ((value & 0x7F) | 0x80);
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64(int32_t value) {
    writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64(uint32_t value) {
    uint8_t tmp[10];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = ((value & 0x7F) | 0x80);
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeFloat(float value) {
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFint32(v);;
}

void ByteArray::writeDouble(double value) {
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}

void ByteArray::writeStringF16(const std::string& value) {
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF32(const std::string& value) {
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF64(const std::string& value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringVint(const std::string& value) {
    writeUint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string& value) {
    write(value.c_str(), value.size());
}

int8_t ByteArray::readFint8() {
    int8_t value;
    read(&value, sizeof(value));
    return value;
}

uint8_t ByteArray::readFuint8() {
    uint8_t value;
    read(&value, sizeof(value));
    return value;
}

#define XX(type) \
    type value; \
    read(&value, sizeof(value)); \
    if(m_endian == SEAICE_BYTE_ORDER) { \
        return value; \
    } else { \
        return byteswap(value); \
    }

int16_t ByteArray::readFint16() {
    XX(int16_t);
}

uint16_t ByteArray::readFuint16() {
    XX(uint16_t);
}

int32_t ByteArray::readFint32() {
    XX(int32_t);
}

uint32_t ByteArray::readFuint32() {
    XX(uint32_t);
}

int64_t ByteArray::readFint64() {
    XX(int64_t);
}

uint64_t ByteArray::readFuint64() {
    XX(uint64_t);
}
#undef XX

int32_t ByteArray::readInt32() {
    return DecodeZigzag32(readUint32());
}

uint32_t ByteArray::readUint32() {
    uint32_t value = 0; //没有初始化导致一直出错
    for(int i = 0; i < 32; i+=7) {
        uint8_t tmp = readFuint8();
        value |= ((uint32_t)(tmp & 0x7F) << i);
        if(tmp < 0x80) {
            break;
        }
    }
    return value;
}

int64_t ByteArray::readInt64() {
    return DecodeZigzag64(readUint64());
}

uint64_t ByteArray::readUint64() {
    uint64_t value = 0;
    for(int i = 0; i < 64; i+=7) {
        uint8_t tmp = readFuint8();
        value |= ((uint64_t)(tmp & 0x7F) << i);
        if(tmp < 0x80) {
            break;
        }
    }
    return value;
}

float ByteArray::readFloat() {
    float ret;
    uint32_t value = readUint32();
    memcpy(&ret, &value, sizeof(value));
    return ret;
}

double ByteArray::readDouble() {
    double ret;
    uint64_t value = readUint64();
    memcpy(&ret, &value, sizeof(value));
    return ret;
}

std::string ByteArray::readStringF16() {
    uint16_t size = readFuint16();
    std::string str;
    str.resize(size);
    read(&str[0], size);
    return str;
}

std::string ByteArray::readStringF32() {
    uint32_t size = readFuint32();
    std::string str;
    str.resize(size);
    read(&str[0], size);
    return str;
}

std::string ByteArray::readStringF64() {
    uint64_t size = readFuint64();
    std::string str;
    str.resize(size);
    read(&str[0], size);
    return str;
}

std::string ByteArray::readStringVint() {
    uint64_t size = readUint64();
    std::string str;
    str.resize(size);
    read(&str[0], size);
    return str;
}

void ByteArray::clear() {
    m_pos = m_size = 0;
    m_cap = m_baseSize;
    Node* tmp = m_root->next;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
    m_cur = m_root;
    m_root->next = nullptr;
}

void ByteArray::write(const void* buf, size_t size) {

    addCapacity(size);

    size_t nPos = m_pos % m_baseSize;
    size_t nCap = 0;
    size_t bPos = 0;
    while(size > 0) {
        nCap = m_cur->size - nPos;
        if(nCap > size) {
            memcpy(m_cur->ptr + nPos, (const char*)buf + bPos, size);
            m_pos += size;
            m_size += size;
            size = 0;
        } else {
            memcpy(m_cur->ptr + nPos, (const char*)buf + bPos, nCap);
            bPos += nCap;
            size -= nCap;
            m_pos += nCap;
            m_size += nCap;
            nPos = 0;
            if(size != 0) {
                m_cur = m_cur->next;
                SEAICE_ASSERT(m_cur);
            }
        }
    }
}

void ByteArray::read(void* buf, size_t size) {

    if(getReadSize() < size) {
        throw std::out_of_range("read not enough len");
    }

    size_t nPos = m_pos % m_baseSize;
    size_t nCap = 0;
    size_t bPos = 0;
    while(size > 0) {
        nCap = m_cur->size - nPos;
        if(nCap > size) {
            memcpy((char*)buf + bPos, m_cur->ptr + nPos, size);
            m_pos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bPos, m_cur->ptr + nPos, nCap);
            bPos += nCap;
            size -= nCap;
            m_pos += nCap;
            nPos = 0;
            m_cur = m_cur->next;
        }
    }
}

void ByteArray::read(void* buf, size_t size, size_t pos) const {

    if((m_size - pos) < size) {
        throw std::out_of_range("read not enough len");
    }

    Node* cur = m_root;
    size_t count = pos / m_baseSize;
    //count += (pos % m_baseSize? 1 : 0);
    while(count > 0) {
        cur = cur->next;
        count--;
    }

    size_t nPos = pos % m_baseSize;
    size_t nCap = 0;
    size_t bPos = 0;
    while(size > 0) {
        nCap = cur->size - nPos;
        if(nCap > size) {
            memcpy((char*)buf + bPos, cur->ptr + nPos, size);
            pos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bPos, cur->ptr + nPos, nCap);
            bPos += nCap;
            size -= nCap;
            pos += nCap;
            nPos = 0;
            cur = cur->next;
        }
    }
}

void ByteArray::setPos(size_t v) {
    if(v > m_cap) {
        throw std::out_of_range("set pos out of range");
    }

    if(v > m_size) {
        m_size = v;
    }

    m_pos = v;
    m_cur = m_root;
    size_t count = m_pos / m_baseSize;
    //count += (m_pos % m_baseSize? 1 : 0);
    while(count > 0) {
        m_cur = m_cur->next;
        count--;
    }
}

void ByteArray::setSize(size_t size) {
    if(size > m_cap || m_pos > size) {
        SEAICE_LOG_ERROR(logger) << "set size failed, size > cap";
        return;
    }
    m_size = size;
}

bool ByteArray::writeToFile(const std::string& filename) const {
    std::ofstream ofs;
    ofs.open(filename, std::ios::binary | std::ios::trunc);
    if(!ofs) {
        SEAICE_LOG_ERROR(logger) << "write to file = " + filename <<
            " errno = " << errno << " err str = " << strerror(errno);
        return false;
    }
    size_t read_size = getReadSize();
    size_t pos = m_pos;
    size_t nPos;
    Node* tmp = m_cur;
    while(read_size > 0) {
        nPos = pos % m_baseSize;
        size_t len = (read_size > m_baseSize? m_baseSize : read_size) - nPos;
        ofs.write(tmp->ptr + nPos, len);
        read_size -= len;
        pos += len;
        tmp = tmp->next;
    }
    return true;
}

bool ByteArray::readFromFile(const std::string& filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ios::binary);
    if(!ifs) {
        SEAICE_LOG_ERROR(logger) << "read from file = " + filename <<
            " errno = " << errno << " err str = " << strerror(errno);
        return false;
    }
    std::shared_ptr<char> buff(new char[m_baseSize], [](char* ptr){delete[] ptr;});
    while(!ifs.eof()) {/// 判断是否结束
        ifs.read(buff.get(), m_baseSize);
        write(buff.get(), ifs.gcount());  //gcount 获取实际读取到的长度
    }
    return true;
}

bool ByteArray::isLittleEndian() const {
    if(m_endian == SEAICE_LITTLE_ENDIAN) {
        return true;
    }
    return false;
}
void ByteArray::setIsLittleEndian(bool val) {
    if(val == true) {
        m_endian = SEAICE_LITTLE_ENDIAN;
    } else {
        m_endian = SEAICE_BIG_ENDIAN;
    }
}

std::string ByteArray::toString() const {
    std::string str;
    str.resize(getReadSize());
    if(str.empty()) {
        return str;
    }
    read(&str[0], str.size(), m_pos);
    return str;
}

std::string ByteArray::toHexString() const {
    std::string str = toString();
    std::stringstream ss;
    for(size_t i = 0; i < str.size(); i++) {
        if(i > 0 && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex <<
            (int)(uint8_t)str[i] << " ";
    }
    return ss.str();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const {
    len = len > getReadSize()? getReadSize() : len;
    if(len == 0) {
        return 0;
    }
    size_t nPos = m_pos % m_baseSize;
    size_t nCap = 0;
    Node* cur = m_cur;
    struct iovec iov;
    while(len > 0) {
        nCap = cur->size - nPos;
        if(nCap > len) {
            iov.iov_base = cur->ptr + nPos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + nPos;
            iov.iov_len = nCap;
            len -= nCap;
            nPos = 0;
            cur = cur->next;
        }
        buffers.push_back(iov);
    }
    return buffers.size();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t pos) const {
    size_t read_size = m_cap - pos;
    len = (len > read_size)? read_size : len;
    if(len == 0) {
        return 0;
    }

    Node* cur = m_root;
    size_t count = pos / m_baseSize;
    //count += (pos % m_baseSize? 1 : 0);
    while(count > 0) {
        cur = cur->next;
        count--;
    }

    size_t nPos = pos % m_baseSize;
    size_t nCap = 0;
    struct iovec iov;
    while(len > 0) {
        nCap = cur->size - nPos;
        if(nCap > len) {
            iov.iov_base = cur->ptr + nPos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + nPos;
            iov.iov_len = nCap;
            len -= nCap;
            nPos = 0;
            cur = cur->next;
        }
        buffers.push_back(iov);
    }
    return buffers.size();
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) {
    if(len == 0) {
        return 0;
    }

    addCapacity(len);

    size_t nPos = m_pos % m_baseSize;
    size_t nCap = 0;
    Node* cur = m_cur;
    struct iovec iov;
    memset(&iov, 0, sizeof(iov));
    while(len > 0) {
        nCap = cur->size - nPos;
        if(nCap > len) {
            iov.iov_base = cur->ptr + nPos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + nPos;
            iov.iov_len = nCap;
            nPos = 0;
            len -= nCap;
            cur = cur->next;
        }
        buffers.push_back(iov);
    }
    return buffers.size();
}


void ByteArray::addCapacity(size_t size) {

    size_t cap = getCapacity();

    if(size <= cap) {
        return;
    }
    size_t addSize = size - cap;

    size_t count = addSize / m_baseSize;
    count += (addSize%m_baseSize) ? 1 : 0;
    Node* tmp = m_cur;
    while(tmp->next) {
        tmp=tmp->next;
    }

    Node* first = nullptr;
    while(count > 0) {
        tmp->next = new Node(m_baseSize);
        tmp = tmp->next;
        count--;
        m_cap += m_baseSize;
        if(first == nullptr) {
            first = tmp;
        }
    }
    if(cap == 0) {
        m_cur = first;
    }
}

}