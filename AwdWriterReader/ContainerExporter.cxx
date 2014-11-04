//
//  ContainerExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 04/11/2014.
//
//

#include "ContainerExporter.h"

bool ContainerExporter::handleNodeType( FbxNodeAttribute::EType type ){
    return type == FbxNodeAttribute::eMesh;
}

void ContainerExporter::doExport(FbxNode* pNode){
    
    const char *name = pNode->GetName();
    AWDContainer* awdContainer = new AWDContainer( name, strlen(name) );
    
    CopyNodeTransform( pNode, awdContainer );
    
    
    mAwd->add_scene_block( awdContainer );
    
}