#include "stream.h"

namespace seaice{

int Stream::readFixSize(void* buffer, size_t length) {
    size_t left = length;
    size_t offset = 0;
    while(left > 0) {
        int len = read((char*)buffer + offset, left);
        if(len <= 0) {
            return len;
        }
        left -= len;
        offset += len;
    }
    return length;
}

int Stream::readFixSize(ByteArray::ptr ba, size_t length) {
    size_t left = length;
    while(left > 0) {
        int len = read(ba, left);
        if(len <= 0) {
            return len;
        }
        left -= len;
    }
    return length;
}

int Stream::writeFixSize(const void* buffer, size_t length) {
    int left = length;
    int offset = 0;
    while(left > 0) {
        int len = write((const char*)buffer + offset, left);
        if(len <= 0) {
            return len;
        }
        left -= len;
        offset += len;
    }
    return length;
}

int Stream::writeFixSize(ByteArray::ptr ba, size_t length) {
    int left = length;
    while(left > 0) {
        int len = write(ba, left);
        if(len <= 0) {
            return len;
        }
        left -= len;
    }
    return length;
}

}