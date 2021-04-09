#include "address.h"
#include "endian.h"
#include "log.h"
#include <algorithm>
#include <string.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stddef.h>
#include <sstream>

//https://blog.csdn.net/myvest/article/details/100859333
//sock地址介绍
/*
在IPV4中，Socket有三种地址相关的结构体：struct sockaddr 、struct sockaddr_in、struct in_addr
struct sockaddr {
sa_family_t sa_family;
char sa_data[14];
};
struct sockaddr_in {
　　short int sin_family; // Address family 
　　unsigned short int sin_port; // Port number 
　　struct in_addr sin_addr; // Internet address 
　　unsigned char sin_zero[8];  Same size as struct sockaddr 
};
struct in_addr{
    in_addr_t s_addr;
}

struct sockaddr_in6 {
    sa_family_t sin6_family;    //AF_INET6
    in_port_t sin6_port;        // Transport layer port 
    uint32_t sin6_flowinfo;     // IPv6 flow information
    struct in6_addr sin6_addr;  // IPv6 address 
    uint32_t sin6_scope_id;     // IPv6 scope-id
};

struct in6_addr {
    union {
        uint8_t u6_addr8[16];
        uint16_t u6_addr16[8];
        uint32_t u6_addr32[4];
    } in6_u;

    #define s6_addr                 in6_u.u6_addr8
    #define s6_addr16               in6_u.u6_addr16
    #define s6_addr32               in6_u.u6_addr32
};
*/

namespace seaice {

static Logger::ptr logger = SEAICE_LOGGER("system");

template<typename T>
static T CreateMask(uint32_t bits) {
    return (1 << (sizeof(T) * 8 - bits)) -1;
}

template<typename T>
static uint32_t CountBytes(T value) {
    uint32_t ret = 0;
    while(value) {
        if(value & 0x01) {
            ret++;
        }
        value >>= 1;
    }
    return ret;
}

//Address::Address() {
//}

Address::~Address(){
}

Address::ptr Address::Create(const sockaddr* addr, socklen_t len) {
    if(addr == nullptr) {
        return nullptr;
    }
    Address::ptr result;
    switch(addr->sa_family) {
        case AF_INET: {
            result.reset(new IPv4Address(*(const sockaddr_in*)addr));
        }
        break;
        case AF_INET6: {
            result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
        }
        break;
        default:
            result.reset(new UnknowAddress(*addr));
            break;
    }
    return result;
}

bool Address::Lookup(std::vector<Address::ptr>& vec, std::string host,
        int family, int type, int protocol) {
    int ret = -1;
    struct addrinfo* res;
    struct addrinfo hint;
    struct addrinfo* cur;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = family;
    hint.ai_socktype = type;
    hint.ai_protocol = protocol;
    //hostname:一个主机名或者地址串(IPv4的点分十进制串或者IPv6的16进制串)
    //service：服务名可以是十进制的端口号，也可以是已定义的服务名称，如ftp、http等
    //[xx.xx.xx.xx:8080]
    //[xx.xx.xx.xx]
    //xx.xx.xx.xx:8080
    //xx.xx.xx.xx
    std::string node, service;
    if(!host.empty()) {
        std::size_t lpos = host.find('[');
        std::size_t rpos = host.find(']');
        std::size_t spos = host.find(':');
        if(spos != std::string::npos) {
            if(rpos != std::string::npos && lpos != std::string::npos) {
                service = host.substr(spos + 1, host.size() - spos - 2);
                node = host.substr(lpos + 1, spos - 1);
            } else {
                service = host.substr(spos + 1, host.size() - spos - 1);
                node = host.substr(0, spos);
            }
        } else {
            if(rpos != std::string::npos && lpos != std::string::npos) {
                node = host.substr(lpos + 1, rpos - 1);
            } else {
                node = host;
            }
        }
    }

    const char* n =  node.empty()? host.c_str() : node.c_str();
    const char* s = service.empty()? nullptr : service.c_str();

    SEAICE_LOG_DEBUG(logger) << "node = " << n << " service = " << s;

    ret = getaddrinfo(n, s, &hint, &res);
    if(ret != 0) {
        SEAICE_LOG_ERROR(logger) << "Address::Lookup getaddrinfo host ="
            << host << " failed" <<" errno = " << errno 
            << " str errno = " << gai_strerror(errno);
    }
    for(cur = res; cur != NULL; cur=cur->ai_next) {
        sockaddr* addr = cur->ai_addr;
        socklen_t len = cur->ai_addrlen;
        vec.push_back(Address::Create(addr, len));
    }
    freeaddrinfo(res);
    return !vec.empty();
}

Address::ptr Address::LookupAny(std::string host, int family, int type,
        int protocol) {
    std::vector<Address::ptr> vec;
    if(Lookup(vec, host, family, type, protocol) == true) {
        return vec.at(0);
    }
    return nullptr;
}

std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(std::string host, int family, int type,
        int protocol) {
    std::vector<Address::ptr> vec;
    if(Lookup(vec, host, family, type, protocol) == true) {
        for(auto& it : vec) {
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(it);//智能指针向下转换
            if(v) {
                return v;
            }
        }
    }
    return nullptr;
}

bool Address::GetInterfaceAddress(std::multimap<std::string, std::pair<Address::ptr, uint32_t> >& result, int family) {
    struct ifaddrs *ifaddr, *cur;
    //char ipstr[128];
    //char netmask[128];
    if(getifaddrs(&ifaddr) == -1) {
        SEAICE_LOG_ERROR(logger) << "getifaddrs failed";
        return false;
    }
    try{
        for(cur = ifaddr; cur != NULL; cur = cur->ifa_next) {
            if(family != AF_UNSPEC && family != cur->ifa_addr->sa_family) {
                continue;
            }
            Address::ptr addr;
            uint32_t prefix_len = ~0u;
            if(family == AF_INET) {
                addr = Create(cur->ifa_addr, sizeof(sockaddr_in));
                uint32_t netmask = ((sockaddr_in*)cur->ifa_netmask)->sin_addr.s_addr;
                prefix_len = CountBytes(netmask);
    /*
                inet_ntop(AF_INET, &(((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr), 
                            ipstr, 16);
                inet_ntop(AF_INET, &(((struct sockaddr_in *)(ifa->ifa_netmask))->sin_addr), 
                            netmask, 16);
                SEAICE_LOG_DEBUG(logger) << "family ip4= " << family;
                SEAICE_LOG_DEBUG(logger) << ipstr << " : " << netmask;
    */
            } else if (family == AF_INET6) {
                addr = Create(cur->ifa_addr, sizeof(sockaddr_in6));
                in6_addr& netmask = ((sockaddr_in6*)cur->ifa_netmask)->sin6_addr;
                prefix_len = 0;
                for(int i = 0; i < 16; i++) {
                    prefix_len += CountBytes(netmask.s6_addr[i]);
                }
                /*
                inet_ntop(AF_INET6, &(((struct sockaddr_in *)(cur->ifa_addr))->sin_addr), 
                            ipstr, 16);
                inet_ntop(AF_INET6, &(((struct sockaddr_in *)(cur->ifa_netmask))->sin_addr), 
                            netmask, 16);
                SEAICE_LOG_DEBUG(logger) << "family ip6 = " << family << ipstr << " : " << netmask;
            */
            }
            if(addr) {
                result.insert(std::make_pair(cur->ifa_name,
                                std::make_pair(addr, prefix_len)));
            }
        }
    } catch (...) {
        SEAICE_LOG_ERROR(logger) << "Address::GetInterfaceAddress exception";
        freeifaddrs(ifaddr);
        return false;
    }
    freeifaddrs(ifaddr);
    return !result.empty();
}

bool Address::GetInterfaceAddress(std::vector<std::pair<Address::ptr, uint32_t> >& result, string iface, int family) {
    if(iface.empty() || iface == "*") {
        if(family == AF_INET) {
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()), 0u));
        } else if(family == AF_INET6) {
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()), 0u));
        }
        return true;
    }

    std::multimap<std::string, std::pair<Address::ptr, uint32_t> > results;
    if(!GetInterfaceAddress(results, family)) {
        return false;
    }

    auto its = results.equal_range(iface);;
    for(; its.first != its.second; ++its.first) {
        result.push_back(its.first->second);
    }
    return !result.empty();
}

int Address::getFamily() const {
    return getAddr()->sa_family;
}

std::string Address::toString() const {
    std::stringstream os;
    insert(os);
    return os.str();
}

bool Address::operator<(const Address& rhs) const {
    const sockaddr* cur = this->getAddr();
    const socklen_t curLen = this->getAddrLen();
    int minLen = std::min(curLen, rhs.getAddrLen());
    int ret = memcmp(cur, rhs.getAddr(), minLen);
    if(curLen == rhs.getAddrLen()) {
        return ret < 0;
    } else if(curLen < rhs.getAddrLen()) {
        if(ret == 0) {
            return true;
        }
    } else {
        if(ret < 0) {
            return true;
        }
    }
    return false;
}

bool Address::operator==(const Address& rhs) const {
    socklen_t curLen = getAddrLen();
    if(curLen != rhs.getAddrLen()) {
        return false;
    }
    const sockaddr* cur = getAddr();
    int ret = memcmp(cur, rhs.getAddr(), curLen);
    return ret == 0;
}

bool Address::operator!=(const Address& rhs) const {
    return !(*this == rhs);
}

IPAddress::IPAddress() {
}

IPAddress::~IPAddress() {
}

IPAddress::ptr IPAddress::Create(const char* address, uint16_t port) {
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(hints));

    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;

    int error = getaddrinfo(address, NULL, &hints, &results);
    if(error) {
        SEAICE_LOG_DEBUG(logger) << "IPAddress::Create(" << address
            << ", " << port << ") error=" << error
            << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }

    try{
        IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
                Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
        if(result) {
            result->setPort(port);
        }
        freeaddrinfo(results);
        return result;
    } catch (...) {
        freeaddrinfo(results);
        return nullptr;
    }
}

IPv4Address::ptr IPv4Address::Create(const char* address, uint16_t port) {
    IPv4Address::ptr rt(new IPv4Address);
    rt->m_addr.sin_port = byteswapOnLittleEndian(port);
    rt->m_addr.sin_family = AF_INET;
    int ret = inet_pton(AF_INET, address, &rt->m_addr.sin_addr);
    if(ret != 1) {
        SEAICE_LOG_ERROR(logger) << "inet_pton failed";
        return nullptr;
    }
    return rt;
}

IPv4Address::IPv4Address(){
}

IPv4Address::IPv4Address(const sockaddr_in& address) {
    m_addr = address;
}

IPv4Address::IPv4Address(uint32_t address, uint32_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapOnLittleEndian(port);
    m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
}

IPv4Address::~IPv4Address() {
}

sockaddr* IPv4Address::getAddr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* IPv4Address::getAddr() const{
    return (sockaddr*)&m_addr;
}

socklen_t IPv4Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream& IPv4Address::insert(std::ostream& os) const {
    uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
    os  <<((addr >> 24) & 0xff) << "."
        <<((addr >> 16) & 0xff) << "."
        <<((addr >> 8) & 0xff) << "."
        <<(addr & 0xff);
    os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
    return os;
}

IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
    if(prefix_len > 32) {
        return nullptr;
    }
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr |= byteswapOnLittleEndian(
        CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}

IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len) {
    if(prefix_len > 32) {
        return nullptr;
    }
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr &= byteswapOnLittleEndian(
        CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}

IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(
        CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::getPort() {
    return byteswapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint32_t port) {
    m_addr.sin_port = byteswapOnLittleEndian(port);
}

IPv6Address::ptr IPv6Address::Create(const char* address, uint16_t port) {
    IPv6Address::ptr rt(new IPv6Address);
    rt->m_addr.sin6_port = byteswapOnLittleEndian(port);
    rt->m_addr.sin6_family = AF_INET6;
    int ret = inet_pton(AF_INET6, address, &rt->m_addr.sin6_addr);
    if(ret != 1) {
        SEAICE_LOG_ERROR(logger) << "inet_pton failed";
        return nullptr;
    }
    return rt;
}

IPv6Address::IPv6Address() {
}

IPv6Address::IPv6Address(const sockaddr_in6& address) {
    m_addr = address;
}

IPv6Address::IPv6Address(uint8_t address[16], uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    m_addr.sin6_family = AF_INET6;
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

IPv6Address::~IPv6Address() {
}

sockaddr* IPv6Address::getAddr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* IPv6Address::getAddr() const {
    return (sockaddr*)&m_addr;
}

socklen_t IPv6Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream& IPv6Address::insert(std::ostream& os) const {
    os << "[";
    uint16_t* addr = (uint16_t*)m_addr.sin6_addr.s6_addr;
    bool used_zeros = false;
    for(size_t i = 0; i < 8; ++i) {
        if(addr[i] == 0 && !used_zeros) {
            continue;
        }
        if(i && addr[i - 1] == 0 && !used_zeros) {
            os << ":";
            used_zeros = true;
        }
        if(i) {
            os << ":";
        }
        os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
    }

    if(!used_zeros && addr[7] == 0) {
        os << "::";
    }

    os << "]:" << byteswapOnLittleEndian(m_addr.sin6_port);
    return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] |=
        CreateMask<uint8_t>(prefix_len % 8);
    for(int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len) {
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] &=
        CreateMask<uint8_t>(prefix_len % 8);
    for(int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0x00;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.s6_addr[prefix_len /8] =
        ~CreateMask<uint8_t>(prefix_len % 8);

    for(uint32_t i = 0; i < prefix_len / 8; ++i) {
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(subnet));
}

uint32_t IPv6Address::getPort() {
    return byteswapOnLittleEndian(m_addr.sin6_port);
}

void IPv6Address::setPort(uint32_t port) {
    m_addr.sin6_port = byteswapOnLittleEndian(port);
}

static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

UnixAddress::UnixAddress() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const string& path) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;

    if(!path.empty() && path[0] == '\0') {
        --m_length;
    }

    if(m_length > sizeof(m_addr.sun_path)) {
        throw std::logic_error("path too long");
    }
    memcpy(m_addr.sun_path, path.c_str(), m_length);
    m_length += offsetof(sockaddr_un, sun_path);
}

UnixAddress::~UnixAddress() {
}

sockaddr* UnixAddress::getAddr() {
    return (sockaddr*)&m_addr;
}

const sockaddr* UnixAddress::getAddr() const{
    return (sockaddr*)&m_addr;
}

socklen_t UnixAddress::getAddrLen() const{
    return m_length;
}

std::ostream& UnixAddress::insert(std::ostream& os) const {
    if(m_length > offsetof(sockaddr_un, sun_path)
            && m_addr.sun_path[0] == '\0') {
        return os << "\\0" << std::string(m_addr.sun_path + 1,
                m_length - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << m_addr.sun_path;
}

std::string UnixAddress::getPath() {
    std::stringstream ss;
    if(m_length > offsetof(sockaddr_un, sun_path)
            && m_addr.sun_path[0] == '\0') {
        ss << "\\0" << std::string(m_addr.sun_path + 1,
                m_length - offsetof(sockaddr_un, sun_path) - 1);
    } else {
        ss << m_addr.sun_path;
    }
    return ss.str();
}

void UnixAddress::setAddrLen(uint32_t v) {
    m_length = v;
}

UnknowAddress::UnknowAddress() {
    memset(&m_addr, 0, sizeof(m_addr));
}

UnknowAddress::UnknowAddress(int family) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

UnknowAddress::UnknowAddress(const sockaddr& addr) {
    m_addr = addr;
}

UnknowAddress::~UnknowAddress() {
}

sockaddr* UnknowAddress::getAddr() {
    return &m_addr;
}

const sockaddr* UnknowAddress::getAddr() const{
    return &m_addr;
}

socklen_t UnknowAddress::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream& UnknowAddress::insert(std::ostream& os) const {
    os << "[UnknownAddress family=" << m_addr.sa_family << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Address& addr) {
    return addr.insert(os);
}



}