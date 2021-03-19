#ifndef _SEAICE_NONCOPYABLE_H_
#define _SEAICE_NONCOPYABLE_H_

namespace seaice {

class Noncopyable{
public:
    Noncopyable() = default;
    ~Noncopyable() = default;

    Noncopyable& operator = (const Noncopyable& copy) = delete;
    Noncopyable(const Noncopyable& copy) = delete;
    //Noncopyable& operator = (Noncopyable& copy) = delete;
    //Noncopyable(const Noncopyable& copy) = delete;
};

}

#endif