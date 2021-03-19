#ifndef __SEAICE_SINGLETON_H__
#define __SEAICE_SINGLETON_H__

template<typename T>
class Singleton {
public:
    static T* getInstance() {
        static T instace;
        return &instace;
    }
};

#endif