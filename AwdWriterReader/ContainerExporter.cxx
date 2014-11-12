//
//  ContainerExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 04/11/2014.
//
//

#include "ContainerExporter.h"

bool ContainerExporter::isHandleObject( FbxObject *pObj ){
    return pObj->Is<FbxNode>();
}

void ContainerExporter::doExport(FbxObject* pObj){
    
    FbxNode *lNode = (FbxNode*) pObj;
    
    const char *name = lNode->GetName();
    AWDContainer* awdContainer = new AWDContainer( name, strlen(name) );
    
    CopyNodeTransform( lNode, awdContainer );
    
    
    mBlocksMap->Set( lNode, awdContainer );
    mAwd->add_scene_block( awdContainer );
    
}