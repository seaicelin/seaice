#ifndef __SEAICE_LRU_CACHE_H__
#define __SEAICE_LRU_CACHE_H__

namespace seaice{
namespace ds{

/*
1. 创建保存节点为 (K,V) 的链表，存放数据
2. 利用 unorder_map 存储数据
    1.1, K 存储键值
    1.2, V 存储一个链表的迭代器
    1.3，存储时更新链表
3. get/set 函数
*/
class LruCache
{
public:
    LruCache();
    ~LruCache();
    
};


}
}
#endif