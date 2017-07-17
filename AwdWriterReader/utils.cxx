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
    
    // did this evaluate pivot
    //lLocal = CalculateGlobalTransform( pNode );//
    lLocal = pNode->EvaluateLocalTransform();
    
    
    rawMatrix[0] =  lLocal.GetRow(0).mData[0];
    rawMatrix[1] =  lLocal.GetRow(0).mData[1];
    rawMatrix[2] =  lLocal.GetRow(0).mData[2];
    
    rawMatrix[3] =  lLocal.GetRow(1).mData[0];
    rawMatrix[4] =  lLocal.GetRow(1).mData[1];
    rawMatrix[5] =  lLocal.GetRow(1).mData[2];
    
    rawMatrix[6] =  lLocal.GetRow(2).mData[0];
    rawMatrix[7] =  lLocal.GetRow(2).mData[1];
    rawMatrix[8] =  lLocal.GetRow(2).mData[2];
    
    rawMatrix[9] =  lLocal.GetRow(3).mData[0];
    rawMatrix[10] = lLocal.GetRow(3).mData[1];
    rawMatrix[11] = lLocal.GetRow(3).mData[2];
    
//    FbxDouble3 lTranslation = pNode->LclTranslation.Get();
//    FbxDouble3 lRotation    = pNode->LclRotation.Get();
//    FbxDouble3 lScaling     = pNode->LclScaling.Get();
    
    //    FBXSDK_printf( "  %f, %f, %f\n", lTranslation.mData[0], lTranslation.mData[1], lTranslation.mData[2] );
    //    FBXSDK_printf( "  %f, %f, %f\n", lRotation.mData[0], lRotation.mData[1], lRotation.mData[2] );
    //    FBXSDK_printf( "  %f, %f, %f\n", lScaling.mData[0], lScaling.mData[1], lScaling.mData[2] );
    //
    //    FBXSDK_printf( "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
    //                rawMatrix[0] ,
    //                rawMatrix[1] ,
    //                rawMatrix[2] ,
    //                rawMatrix[3] ,
    //                rawMatrix[4] ,
    //                rawMatrix[5] ,
    //                rawMatrix[6] ,
    //                rawMatrix[7] ,
    //                rawMatrix[8] ,
    //                rawMatrix[9] ,
    //                rawMatrix[10],
    //                rawMatrix[11]
    //
    //                  );
    
    
    // todo : check values
    // from max
    // sx, #,  #
    // #,  #,  -sy
    // #,  sz, #
    // tx, tz, -ty
    
    sceneBlock->set_transform( rawMatrix );
    
    // short
    //sceneBlock->set_transform( pNode->EvaluateLocalTransform() );
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

