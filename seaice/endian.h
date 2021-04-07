#ifndef __SEAICE_ENDIAN_H__
#define __SEAICE_ENDIAN_H__

#include <stdint.h>
#include <byteswap.h>

#define SEAICE_LITTLE_ENDIAN 1
#define SEAICE_BIG_ENDIAN 2

/*
https://blog.csdn.net/szchtx/article/details/42834391
*
16位宽的数0x1234，在不同的模式下，存储方式为：
地址       0x4000  0x4001
小端模式    0x34    0x12
大端模式    0x12    0x34

32位宽的数0x12345678，在不同的模式下，存储方式为：
地址       0x4000  0x4001  0x4002  0x4003
小端模式    0x78    0x56    0x34    0x12
大端模式    0x12    0x34    0x56    0x78
*
使用联合，通过判断首个成员的值，确定是大端还是小端模式：
bool IsBigEndian(){  
    union NUM{
        int a;  
        char b;  
    }num;  
    num.a = 0x1234;  
    if( num.b == 0x12 ){  
        return true;  
    }   
    return false;  
}
*/
namespace seaice {

#if BYTE_ORDER == BIG_ENDIAN
#define SEAICE_BYTE_ORDER SEAICE_BIG_ENDIAN
#else
#define SEAICE_BYTE_ORDER SEAICE_LITTLE_ENDIAN
#endif

template<typename T>
typename std::enable_if<sizeof(uint64_t) == sizeof(T), T>::type 
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

template<typename T>
typename std::enable_if<sizeof(uint32_t) == sizeof(T), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template<typename T>
typename std::enable_if<sizeof(uint16_t) == sizeof(T), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if SEAICE_BYTE_ORDER == SEAICE_BIG_ENDIAN

template<typename T>
T byteswapOnLittleEndian(T t) {
    return t;
}

template<typename T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}

#else

template<typename T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

template<typename T>
T byteswapOnBigEndian(T t) {
    return t;
}

#endif

}
#endif