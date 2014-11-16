//
//  NodeExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 04/11/2014.
//
//

#include "NodeExporter.h"

//
// WIP
// todo : need to find out how export xform correctly (YUP/ZUP, pivots, global xform etc)
//
//

FbxAMatrix CalculateGlobalTransform(FbxNode* pNode)
{
    FbxAMatrix lTranlationM, lScalingM, lScalingPivotM, lScalingOffsetM, lRotationOffsetM, lRotationPivotM, \
    lPreRotationM, lRotationM, lPostRotationM, lTransform;
    
    FbxAMatrix lParentGX, lGlobalT, lGlobalRS;
    
    if(!pNode)
    {
        lTransform.SetIdentity();
        return lTransform;
    }
    
    // Construct translation matrix
    FbxVector4 lTranslation = pNode->LclTranslation.Get();
    lTranlationM.SetT(lTranslation);
    
    // Construct rotation matrices
    FbxVector4 lRotation = pNode->LclRotation.Get();
    FbxVector4 lPreRotation = pNode->PreRotation.Get();
    FbxVector4 lPostRotation = pNode->PostRotation.Get();
    
    
    lRotationM.SetR(lRotation);
    lPreRotationM.SetR(lPreRotation);
    lPostRotationM.SetR(lPostRotation);
    
    
    lTranslation = lPreRotationM.Inverse().MultT( lTranslation );
    
    // Construct scaling matrix
    FbxVector4 lScaling = pNode->LclScaling.Get();
    lScalingM.SetS(lScaling);
    
    // Construct offset and pivot matrices
    FbxVector4 lScalingOffset = pNode->ScalingOffset.Get();
    FbxVector4 lScalingPivot = pNode->ScalingPivot.Get();
    FbxVector4 lRotationOffset = pNode->RotationOffset.Get();
    FbxVector4 lRotationPivot = pNode->RotationPivot.Get();
    lScalingOffsetM.SetT(lScalingOffset);
    lScalingPivotM.SetT(lScalingPivot);
    lRotationOffsetM.SetT(lRotationOffset);
    lRotationPivotM.SetT(lRotationPivot);
    
    // Calculate the global transform matrix of the parent node
    FbxNode* lParentNode = pNode->GetParent();
    if(lParentNode)
    {
        lParentGX = CalculateGlobalTransform(lParentNode);
    }
    else
    {
        lParentGX.SetIdentity();
    }
    
    //Construct Global Rotation
    FbxAMatrix lLRM, lParentGRM;
    FbxVector4 lParentGR = lParentGX.GetR();
    lParentGRM.SetR(lParentGR);
    lLRM = lPreRotationM * lRotationM * lPostRotationM;
    lLRM = lRotationM;
    
    //Construct Global Shear*Scaling
    //FBX SDK does not support shear, to patch this, we use:
    //Shear*Scaling = RotationMatrix.Inverse * TranslationMatrix.Inverse * WholeTranformMatrix
    FbxAMatrix lLSM, lParentGSM, lParentGRSM, lParentTM;
    FbxVector4 lParentGT = lParentGX.GetT();
    lParentTM.SetT(lParentGT);
    lParentGRSM = lParentTM.Inverse() * lParentGX;
    lParentGSM = lParentGRM.Inverse() * lParentGRSM;
    lLSM = lScalingM;
    
    //Do not consider translation now
    FbxTransform::EInheritType lInheritType = pNode->InheritType.Get();
    if(lInheritType == FbxTransform::eInheritRrSs)
    {
        lGlobalRS = lParentGRM * lLRM * lParentGSM * lLSM;
    }
    else if(lInheritType == FbxTransform::eInheritRSrs)
    {
        lGlobalRS = lParentGRM * lParentGSM * lLRM * lLSM;
    }
    else if(lInheritType == FbxTransform::eInheritRrs)
    {
        FbxAMatrix lParentLSM;
        FbxVector4 lParentLS = lParentNode->LclScaling.Get();
        lParentLSM.SetS(lParentLS);
        
        FbxAMatrix lParentGSM_noLocal = lParentGSM * lParentLSM.Inverse();
        lGlobalRS = lParentGRM * lLRM * lParentGSM_noLocal * lLSM;
    }
    else
    {
        FBXSDK_printf("error, unknown inherit type! \n");
    }
    
    // Construct translation matrix
    // Calculate the local transform matrix
    lTransform = lTranlationM
                * lRotationOffsetM
    * lRotationPivotM
//    * lPreRotationM
    * lRotationM
//    * lPostRotationM
    * lRotationPivotM.Inverse()
    * lScalingOffsetM
    * lScalingPivotM
    * lScalingM
    * lScalingPivotM.Inverse();
    
    
    FbxVector4 lLocalTWithAllPivotAndOffsetInfo = lTransform.GetT();
    // Calculate global translation vector according to:
    // GlobalTranslation = ParentGlobalTransform * LocalTranslationWithPivotAndOffsetInfo
    FbxVector4 lGlobalTranslation = lParentGX.MultT(lLocalTWithAllPivotAndOffsetInfo);
    lGlobalT.SetT(lGlobalTranslation);
    
    //Construct the whole global transform
    lTransform = lGlobalT * lGlobalRS;
    
    return lTransform;
}



void NodeExporter::setup( ExportContext *context, ExporterProvider *provider )
{
    mContext = context;
    mExporters = provider;
}


void NodeExporter::release( )
{
    mContext = NULL;
}


void CopyNodeTransform( FbxNode* pNode, AWDSceneBlock* sceneBlock )
{
    FbxAMatrix lLocal;
    
    
    
    static double rawMatrix[12];
    
    // did this evaluate pivot
    lLocal = CalculateGlobalTransform( pNode );//pNode->EvaluateLocalTransform();
    
    
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
    
    FbxDouble3 lTranslation = pNode->LclTranslation.Get();
    FbxDouble3 lRotation    = pNode->LclRotation.Get();
    FbxDouble3 lScaling     = pNode->LclScaling.Get();
    
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
//    sceneBlock->set_transform( pNode->EvaluateLocalTransform() );
}




// ================
// ExporterProvider


ExporterLinkedItem::ExporterLinkedItem(NodeExporter *exporter )
{
    mExporter = exporter;
    mNext = NULL;
}

ExporterLinkedItem::~ExporterLinkedItem()
{
    mExporter = NULL;
    mNext = NULL;
}


ExporterLinkedItem* ExporterLinkedItem::next()
{
    return mNext;
}

void ExporterLinkedItem::setNext( ExporterLinkedItem* item )
{
    if( mNext ) {
        item->setNext( mNext );
    }
    mNext = item;
}

NodeExporter* ExporterLinkedItem::getExporter()
{
    return mExporter;
}






ExporterProvider::ExporterProvider( NodeExporter* pDefault )
{
    mDefault = pDefault;
    mHead = NULL;
    mTail = NULL;
}

ExporterProvider::~ExporterProvider( )
{
    ExporterLinkedItem *cur = mHead;
    while (cur) {
        ExporterLinkedItem *next = cur->next();
        delete cur;
        cur = next;
    }
    mHead = mTail = NULL;
}

void ExporterProvider::addExporter(NodeExporter *exporter)
{
    ExporterLinkedItem *item = new ExporterLinkedItem( exporter );
    if( mTail ) {
        mTail->setNext(item);
    } else {
        mHead = mTail = item;
    }
}

NodeExporter* ExporterProvider::findExporter( FbxObject* pObj )
{
    ExporterLinkedItem *item = mHead;
    
    while( item ) {
        
        if( item->getExporter()->isHandleObject( pObj ) ){
            return item->getExporter();
        }
        
        item = item->next();
    }
    
    // no suitable exporter found
    return NULL;
}



NodeExporter* ExporterProvider::getDefaultExporter()
{
    return mDefault;
}

