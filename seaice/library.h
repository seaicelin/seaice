#ifndef __SEAICE_LIBRARY_H__
#define __SEAICE_LIBRARY_H__

#include "module.h"

/*
主要用来加载指定路径的动态链接库
通过GetModule生成 module  对象，
定义Module的时候指定释放函数。
释放的时候需要析构和关闭动态链接库。

dlopen  --> 打开动态链接库
dlsym   --> 调用链接库指定函数
dlclose --> 关闭动态链接库。

*/
namespace seaice{

class Library
{
public:
    static Module::ptr GetModule(const std::string& path);
};

}
#endif