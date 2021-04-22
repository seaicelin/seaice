#ifndef __SEAICE_SINGLETON_H__
#define __SEAICE_SINGLETON_H__

template<typename T>
class Singleton {
public:
    static T* GetInstance() {
        static T instace;
        return &instace;
    }
};

#endif