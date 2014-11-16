//
//  ContainerExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 04/11/2014.
//
//

#include "ContainerExporter.h"
#include "utils.h"

bool ContainerExporter::isHandleObject( FbxObject *pObj ){
    return pObj->Is<FbxNode>();
}

void ContainerExporter::doExport(FbxObject* pObj){
    
    FbxNode *lNode = (FbxNode*) pObj;
    
    const char *name = lNode->GetName();
    AWDContainer* awdContainer = new AWDContainer( name, static_cast<unsigned short>(strlen(name)) );
    
    AwdUtils::CopyNodeTransform( lNode, awdContainer );
    
    AWDSceneBlock *parent = (AWDSceneBlock*) mContext->GetBlocksMap()->Get( lNode->GetParent() );
    awdContainer->set_parent( parent );
    
    mContext->add_scene_block( awdContainer, lNode );
    
}