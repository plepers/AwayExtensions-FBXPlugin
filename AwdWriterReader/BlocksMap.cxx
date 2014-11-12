//
//  BlocksMap.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 05/11/2014.
//
//

#include "BlocksMap.h"


BlocksMap::BlocksMap(bool weakReference)
{
    firstItem = NULL;
    lastItem = NULL;
    this->weakReference=weakReference;
}
BlocksMap::~BlocksMap()
{
    BlocksMapItem *cur = firstItem;
    while (cur) {
        BlocksMapItem *next = cur->next;
        if(!this->weakReference)
            delete cur->block;
        free(cur);
        cur = next;
    }
    
    firstItem = lastItem = NULL;
}
void BlocksMap::Set(void *key, AWDBlock *val)
{
    BlocksMapItem *item;
    
    item = (BlocksMapItem *)malloc(sizeof(BlocksMapItem));
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
AWDBlock *BlocksMap::Get(void *key)
{
    BlocksMapItem *cur;
    
    cur = firstItem;
    while (cur) {
        if (cur->key == key)
            return cur->block;
        
        cur = cur->next;
    }
    
    return NULL;
}
void *BlocksMap::GetKeyByVal(AWDBlock *val)
{
    BlocksMapItem *cur;
    
    cur = firstItem;
    while (cur) {
        if (cur->block == val)
            return cur->key;
        
        cur = cur->next;
    }
    
    return NULL;
}
void BlocksMap::DeleteVals()
{
    BlocksMapItem *cur;
    
    cur = firstItem;
    while (cur) {
        delete cur->block;
        cur = cur->next;
    }
}

