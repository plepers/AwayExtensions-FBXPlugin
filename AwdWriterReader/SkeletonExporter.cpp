//
//  SkeletonExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 17/11/2014.
//
//

#include "SkeletonExporter.h"
#include "utils.h"

bool SkeletonExporter::isHandleObject( FbxObject *pObj ){
    if( pObj->Is<FbxNode>() ){
        FbxNode * lNode = (FbxNode*) pObj;
        FbxNodeAttribute::EType nType = (lNode->GetNodeAttribute()->GetAttributeType());
        return nType == FbxNodeAttribute::eSkeleton;
    }
    return false;
}

void SkeletonExporter::doExport(FbxObject* pObj){
    
    FbxNode *lNode = (FbxNode*) pObj;
    
    
    FbxSkeleton *skeleton = lNode->GetSkeleton();
    
    
    AWDSkeletonJoint *joint = new AWDSkeletonJoint( NULL, 0, NULL );
    AwdUtils::CopyNodeName( pObj, joint );
    
    if( skeleton->GetSkeletonType() == FbxSkeleton::eRoot )
    {
        // this is the root node of skeleton
        // create the AWDSkeleton here
        
        
    }
    
    const char *name = lNode->GetName();
    AWDContainer* awdContainer = new AWDContainer( name, static_cast<unsigned short>(strlen(name)) );
    
    AwdUtils::CopyNodeTransform( lNode, awdContainer );
    
    AWDSceneBlock *parent = (AWDSceneBlock*) mContext->GetBlocksMap()->Get( lNode->GetParent() );
    awdContainer->set_parent( parent );
    
    mContext->add_scene_block( awdContainer, lNode );
    
}