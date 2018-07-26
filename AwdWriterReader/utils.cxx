//
//  utils.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 16/11/2014.
//
//

#include "utils.h"


void AwdUtils::CopyNodeName( FbxObject* pObj, AWDNamedElement* block )
{
    const char *name = pObj->GetName();
    char *bName = (char*) malloc(strlen( name ) + 1);
    strcpy( bName, name );
    block->set_name( bName, static_cast<unsigned short>( strlen( bName ) ) );
}


void AwdUtils::FbxMatrixTo4x3( FbxAMatrix* fbxMat, double* raw4x3 )
{
    
    raw4x3[0] =  fbxMat->GetRow(0).mData[0];
    raw4x3[1] =  fbxMat->GetRow(0).mData[1];
    raw4x3[2] =  fbxMat->GetRow(0).mData[2];
    
    raw4x3[3] =  fbxMat->GetRow(1).mData[0];
    raw4x3[4] =  fbxMat->GetRow(1).mData[1];
    raw4x3[5] =  fbxMat->GetRow(1).mData[2];
    
    raw4x3[6] =  fbxMat->GetRow(2).mData[0];
    raw4x3[7] =  fbxMat->GetRow(2).mData[1];
    raw4x3[8] =  fbxMat->GetRow(2).mData[2];
    
    raw4x3[9] =  fbxMat->GetRow(3).mData[0];
    raw4x3[10] = fbxMat->GetRow(3).mData[1];
    raw4x3[11] = fbxMat->GetRow(3).mData[2];

}


awd_float64* AwdUtils::mtx4x3_identity(awd_float64 *mtx)
{
    if (mtx == NULL) {
        mtx = (awd_float64*)malloc(16 * sizeof(awd_float64));
    }
    
    mtx[0]  = 1.0; mtx[1]  = 0.0; mtx[2]  = 0.0;
    mtx[3]  = 0.0; mtx[4]  = 1.0; mtx[5]  = 0.0;
    mtx[6]  = 0.0; mtx[7]  = 0.0; mtx[8]  = 1.0;
    mtx[9]  = 0.0; mtx[10] = 0.0; mtx[11] = 0.0;
    
    return mtx;
}



void AwdUtils::CopyNodeTransform( FbxNode* pNode, AWDSceneBlock* sceneBlock )
{
    
    FbxAMatrix lLocal;

    double *rawMatrix  = new double[ 12 ];
    
    lLocal = pNode->EvaluateLocalTransform( FBXSDK_TIME_INFINITE, FbxNode::eSourcePivot );
    
    AwdUtils::FbxMatrixTo4x3( &lLocal, rawMatrix );
    
    sceneBlock->set_transform( rawMatrix );
    
}


bool AwdUtils::isMatrix2dIdentity( awd_float64 *mtx )
{
    return !(
        mtx[0] != 1.0 ||
        mtx[1] != 0.0 ||
        mtx[2] != 0.0 ||
        mtx[3] != 1.0 ||
        mtx[4] != 0.0 ||
        mtx[5] != 0.0
    );
}

