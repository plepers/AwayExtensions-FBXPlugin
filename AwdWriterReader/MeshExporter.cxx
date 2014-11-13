//
//  MeshExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 05/11/2014.
//
//

#include "MeshExporter.h"
#include "GeomExporter.h"



bool MeshExporter::isHandleObject( FbxObject *pObj ){
    if( pObj->Is<FbxNode>() ){
        FbxNode * lNode = (FbxNode*) pObj;
        FbxNodeAttribute::EType nType = (lNode->GetNodeAttribute()->GetAttributeType());
        return nType == FbxNodeAttribute::eMesh;
    }
    return false;
}

void MeshExporter::doExport(FbxObject* pObj){
    
    FbxNode *pNode = (FbxNode*) pObj;
    
    
    //
    // export geometry
    //
    AWDBlock *geomBlock = NULL;
    
    FbxMesh *lMesh = pNode->GetMesh();
    
    if( lMesh ){
        geomBlock = mBlocksMap->Get( lMesh );
        
        if( !geomBlock ) {
            
            GeomExporter* gExporter = new GeomExporter();
            gExporter->setup( mAwd, mFbxManager, mBlocksMap );
            gExporter->doExport( lMesh );
            gExporter->release();
            
        }
        
        geomBlock = mBlocksMap->Get( lMesh );
        
        if( geomBlock == NULL ){
            FBXSDK_printf( "WARN : geom not found/exported, mesh exported without geometry! \n" );
        }

    }
    
    
    
    
    
    
    
    
    
    const char *name = pNode->GetName();
    AWDMeshInst* awdMesh = new AWDMeshInst( name, strlen(name), geomBlock );
    
    CopyNodeTransform( pNode, awdMesh );
    
    
    mAwd->add_mesh_data( awdMesh );
    
}