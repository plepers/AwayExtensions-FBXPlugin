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
public:
    
    void exportAnimator( FbxSkin*, FbxMesh* mesh );
    
    AWDSkeleton* exportSkeleton( FbxSkin* );
    AWDAnimationSet* exportAnimationSet( FbxSkin *skin );
    AWDSkeletonAnimation* exportAnimation( FbxAnimStack* animStack, FbxSkin *skin );
    
    AWDSkeletonPose* CreateSkeletonPoseAtTime( FbxTime time, FbxSkin *skin );
    
    FbxTimeSpan getSkinAnimationTimespan( FbxAnimStack* animStack, FbxSkin *skin );
};


#endif /* defined(__FbxAwdExporter__SkeletonExporter__) */
