//
//  BlocksMap.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 05/11/2014.
//
//

#ifndef __FbxAwdExporter__BlocksMap__
#define __FbxAwdExporter__BlocksMap__


#include "awd.h"

/**
 * BlockCache
 * Cache that is used generically to store any type of block or
 * sub-element in a look-up table by FBX object.
 */
template <typename T>
struct BlocksMapItem {
    void *key;
    T *block;
    BlocksMapItem<T> *next;
};




template <typename T>
class BlocksMap
{
private:
    BlocksMapItem<T> *firstItem;
    BlocksMapItem<T> *lastItem;
    bool weakReference;
    
public:
    BlocksMap(bool weakReference = true);
    ~BlocksMap();
    
    void        Set( void *key, T *block );
    T           *Get( void *key );
    void        *GetKeyByVal( T *val );
    void        DeleteVals();
};





template <typename T>
BlocksMap<T>::BlocksMap(bool weakReference)
{
    firstItem = NULL;
    lastItem = NULL;
    this->weakReference=weakReference;
}

template <typename T>
BlocksMap<T>::~BlocksMap()
{
    BlocksMapItem<T> *cur = firstItem;
    while (cur) {
        BlocksMapItem<T> *next = cur->next;
        if(!this->weakReference)
            delete cur->block;
        free(cur);
        cur = next;
    }
    
    firstItem = lastItem = NULL;
}


template <typename T>
void BlocksMap<T>::Set(void *key, T *val)
{
    BlocksMapItem<T> *item;
    
    item = (BlocksMapItem<T> *)malloc(sizeof(BlocksMapItem<T>));
    item->key   = key;
    item->block = val;
    item->next  = NULL;
    
    if (!firstItem) {
        firstItem = item;
    }
    else {
        lastItem->next = item;
    }
    
    lastItem = item;
}

template <typename T>
T* BlocksMap<T>::Get(void *key)
{
    BlocksMapItem<T> *cur;
    
    cur = firstItem;
    while (cur) {
        if (cur->key == key)
            return cur->block;
        
        cur = cur->next;
    }
    
    return NULL;
}

template <typename T>
void* BlocksMap<T>::GetKeyByVal(T *val)
{
    BlocksMapItem<T> *cur;
    
    cur = firstItem;
    while (cur) {
        if (cur->block == val)
            return cur->key;
        
        cur = cur->next;
    }
    
    return NULL;
}

template <typename T>
void BlocksMap<T>::DeleteVals()
{
    BlocksMapItem<T> *cur;
    
    cur = firstItem;
    while (cur) {
        delete cur->block;
        cur = cur->next;
    }
}


#endif /* defined(__FbxAwdExporter__BlocksMap__) */
