//
//  BlocksMap.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 05/11/2014.
//
//

#ifndef __FbxAwdExporter__BlocksMap__
#define __FbxAwdExporter__BlocksMap__


#include <awd/awd.h>

/**
 * BlockCache
 * Cache that is used generically to store any type of block or
 * sub-element in a look-up table by MAX object.
 */
typedef struct BlocksMapItem_struct {
    void *key;
    AWDBlock *block;
    BlocksMapItem_struct *next;
} BlocksMapItem;

class BlocksMap
{
private:
    BlocksMapItem *firstItem;
    BlocksMapItem *lastItem;
    bool weakReference;
    
public:
    BlocksMap(bool weakReference = true);
    ~BlocksMap();
    
    void        Set(void *key, AWDBlock *block);
    AWDBlock    *Get(void *key);
    void        *GetKeyByVal(AWDBlock *val);
    void        DeleteVals();
};

#endif /* defined(__FbxAwdExporter__BlocksMap__) */
