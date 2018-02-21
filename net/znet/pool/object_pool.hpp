#ifndef _OBJECT_POOL_HPP
#define _OBJECT_POOL_HPP

#include <atomic>
#include "AtomicLinkedList.h"

template <class T>
    class ObjectPool
    {
    public:
        ObjectPool(size_t size = 10, int grow = 20) : grow_(grow)
        {
            for(int i = 0; i < size; i++)
            {
                T * pObj = new T();
                obj_list_.insertHead(pObj);
            }
        }

        T * Acquire()
        {
            alloc_count_++;
            return Alloc();
        }

        void Release(T * pObj)
        {
            release_count_++;
            pObj->~T();
            obj_list_.insertHead(pObj);
            pObj = nullptr;
        }

    private:
        T * Alloc()
        {
            if (obj_list_.empty())
            {
                for(int i = 0; i < grow_; i++)
                {
                    T* pObj = new T();
                    obj_list_.insertHead(pObj);
                }
            }
            T * pItem = nullptr;
            obj_list_.sweep([&](T* pObj){
                    pItem = pObj ;
                    });
            return pItem;
        }

        AtomicLinkedList< T* > obj_list_;
        std::atomic_int alloc_count_;
        std::atomic_int release_count_;
        int grow_;
    };
#endif //


