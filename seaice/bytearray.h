#ifndef __SEAICE_BYTEARRAY_H__
#define __SEAICE_BYTEARRAY_H__

#include <stdint.h>
#include <memory>
#include <vector>
#include <sys/socket.h>

namespace seaice {

class ByteArray {
public:
    typedef std::shared_ptr<ByteArray> ptr;

struct Node{
    Node();
    Node(size_t len);
    ~Node();
    int size;
    char* ptr;
    Node* next;
};

//ByteArray();
ByteArray(size_t baseSize = 4096);
~ByteArray();

//编码
//https://blog.csdn.net/weixin_43708622/article/details/111397290

void writeFint8(int8_t value);
void writeFuint8(uint8_t value);
void writeFint16(int16_t value);
void writeFuint16(uint16_t value);
void writeFint32(int32_t value);
void writeFuint32(uint32_t value);
void writeFint64(int64_t value);
void writeFuint64(uint64_t value);
void writeInt32(int32_t value);
void writeUint32(uint32_t value);
void writeInt64(int32_t value);
void writeUint64(uint32_t value);
void writeFloat(float value);
void writeDouble(double value);
void writeStringF16(const std::string& value);
void writeStringF32(const std::string& value);
void writeStringF64(const std::string& value);
void writeStringVint(const std::string& value);
void writeStringWithoutLength(const std::string& value);

int8_t readFint8();
uint8_t readFuint8();
int16_t readFint16();
uint16_t readFuint16();
int32_t readFint32();
uint32_t readFuint32();
int64_t readFint64();
uint64_t readFuint64();
int32_t readInt32();
uint32_t readUint32();
int64_t readInt64();
uint64_t readUint64();
float readFloat();
double readDouble();
std::string readStringF16();
std::string readStringF32();
std::string readStringF64();
std::string readStringVint();
std::string readStringWithoutLength();

void clear();
void write(const void* buf, size_t size);
void read(void* buf, size_t size);

size_t getPos() const {return m_pos;}
void setPos(size_t v);
bool writeToFile(const std::string& filename) const;
bool readFromFile(const std::string& filename);
size_t getBaseSize() const {return m_baseSize;}
size_t getReadSize() const {return m_size - m_pos;}
bool isLittleEndian() const;
void setIsLittleEndian(bool val);

std::string toString() const;
std::string toHexString() const;

uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;
uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t pos) const;
uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

size_t getSize() const {return m_size;}
void setSize(size_t size);
private:
    void addCapacity(size_t size);
    size_t getCapacity() const {return m_cap - m_pos;};
    void read(void* buf, size_t size, size_t pos) const;
private:
    size_t m_baseSize;
    size_t m_pos;
    size_t m_cap;
    size_t m_size;
    int8_t m_endian;
    Node* m_root;
    Node* m_cur;
};

}
#endif