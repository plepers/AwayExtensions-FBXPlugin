//
//  SkeletonExporter.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 17/11/2014.
//
//

#ifndef __FbxAwdExporter__SkeletonExporter__
#define __FbxAwdExporter__SkeletonExporter__


#include "NodeExporter.h"


class SkeletonExporter : public NodeExporter
{
    virtual bool isHandleObject( FbxObject* );
    
    virtual void doExport( FbxObject* );
};


#endif /* defined(__FbxAwdExporter__SkeletonExporter__) */
