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
    // export Geometry
    //
    
    AWDBlock *geomBlock = NULL;
    
    FbxMesh *lMesh = pNode->GetMesh();
    
    if( lMesh ){
        GeomExporter* gExporter = new GeomExporter();
        gExporter->setup( mContext, mExporters );
        gExporter->doExport( lMesh );
        gExporter->release();
        
        // call GetMesh again after geom export since
        // geom exporter can replace the actual Mesh attribute
        // (Triangulation)
        lMesh = pNode->GetMesh();
        
        geomBlock = mContext->GetBlocksMap()->Get( lMesh );
        
        if( geomBlock == NULL ){
            FBXSDK_printf( "WARN : geom not found/exported, mesh exported without geometry! \n" );
        }

    }
    
    // export MeshInstance
    //
    const char *name = pNode->GetName();
    AWDMeshInst* awdMesh = new AWDMeshInst( name, static_cast<unsigned short>(strlen(name)), geomBlock );
    
    CopyNodeTransform( pNode, awdMesh );
    
    
    
    // retreive materials
    // 
    // materials should have already been exported
    // by GeomExporter. Just retreive them from cache.
    
    AWDBlockList *materialList = new AWDBlockList();
    
    int numMaterials = pNode->GetMaterialCount();
    
    for ( int matIndex = 0; matIndex < numMaterials; matIndex++ )
    {
        AWDBlock *matBlock = mContext->GetBlocksMap()->Get(pNode->GetMaterial( matIndex ) );
        materialList->append( matBlock );
        awdMesh->add_material( (AWDMaterial*)matBlock );
    }
    
    awdMesh->set_defaultMat(materialList->getByIndex(0));
    awdMesh->set_pre_materials( materialList );
    
    // add to awd
    //
    mContext->add_mesh_data( awdMesh, pNode );
    
}