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


AWDSkeleton* SkeletonExporter::exportSkeleton( FbxSkin *skin ){
    
    // name seems empty...
    const char *name = skin->GetName();
    AWDSkeleton *skeleton = new AWDSkeleton( name, static_cast<unsigned short>(strlen(name)), 0 );
    
    FBXSDK_printf( "export Skin %s \n", name );
    
    
    int lClusterCount = skin->GetClusterCount();
    
    
    int lClusterIndex;
    
    FbxAMatrix lMeshTransform;
    FbxAMatrix lLinkTransform;
    
    
    
    // create a "dummy" root joint and flatten all joints in it
    
    AWDSkeletonJoint* rootAwdJoint = new AWDSkeletonJoint( "rootJoint", static_cast<unsigned short>( strlen("rootJoint") ), AwdUtils::mtx4x3_identity( NULL ), lClusterCount+1 );
    skeleton->set_root_joint( rootAwdJoint );
    
    
    BlocksMap<AWDSkeletonJoint> *lJointMap = new BlocksMap<AWDSkeletonJoint>();
    
    
    for( lClusterIndex = 0; lClusterIndex != lClusterCount; ++lClusterIndex )
    {
        
        FbxCluster* lCluster = skin->GetCluster( lClusterIndex );
        
        FBX_ASSERT( lCluster->GetLinkMode() === FbxCluster::eNormalize );
        
        FbxNode* joint       = lCluster->GetLink();
        FbxNode* pjoint      = joint->GetParent();
        FbxString linkName   = joint->GetName();
        FbxString pName      = pjoint->GetName();
        
        
//        FBXSDK_printf( "link name %s \n", linkName.Buffer() );
//        FBXSDK_printf( "link mode %i \n", lCluster->GetLinkMode() );
//        FBXSDK_printf( "link parent name %s \n", pName.Buffer() );
        
        FbxAMatrix jointLocalTransform = joint->EvaluateLocalTransform( FBXSDK_TIME_ONE_SECOND );
        FbxAMatrix jointGlobalTransform = joint->EvaluateGlobalTransform( FBXSDK_TIME_ONE_SECOND );
        
        
        
        // binding Matrix
        double *lAwdLinkTransform  = new double[ 12 ];
        
        lCluster->GetTransformMatrix( lMeshTransform );
        lCluster->GetTransformLinkMatrix( lLinkTransform );
        
        AwdUtils::FbxMatrixTo4x3( &lLinkTransform, lAwdLinkTransform );
//        AwdUtils::FbxMatrixTo4x3( &jointGlobalTransform, lAwdLinkTransform );
        
        
        
        
        AWDSkeletonJoint* awdJoint = new AWDSkeletonJoint( linkName, static_cast<unsigned short>( strlen(linkName) ), lAwdLinkTransform, lClusterIndex+1 );
        
        lJointMap->Set( joint, awdJoint );
        
        
        // reparenting
        // -----------
        AWDSkeletonJoint* parentJoint = lJointMap->Get( pjoint );
        
        if( parentJoint == NULL ){
            parentJoint = rootAwdJoint;
        }
        
        parentJoint->add_child_joint( awdJoint );
        
    }
    
    
    mContext->GetAwd()->add_skeleton( skeleton );
    
    return skeleton;
    
}



AWDSkeletonPose* SkeletonExporter::CreateSkeletonPoseAtTime( FbxTime time, FbxSkin *skin ){
    
    int lClusterCount = skin->GetClusterCount();
    
    AWDSkeletonPose* pose = new AWDSkeletonPose( "", 0);
    
    FbxAMatrix lLinkTransform;
    
    for( int lClusterIndex = 0; lClusterIndex != lClusterCount; ++lClusterIndex )
    {
        FbxCluster* lCluster = skin->GetCluster( lClusterIndex );
        FbxNode* joint       = lCluster->GetLink();
        
        lLinkTransform = joint->EvaluateGlobalTransform( time );
        
        double *lAwdLinkTransform  = new double[ 12 ];
        AwdUtils::FbxMatrixTo4x3( &lLinkTransform, lAwdLinkTransform );
        pose->set_next_transform( lAwdLinkTransform );
    }
    
    // dummy root node is last
    pose->set_next_transform( AwdUtils::mtx4x3_identity( NULL ) );
    
    
    mContext->GetAwd()->add_skeleton_pose( pose );
    
    return pose;
}



FbxTimeSpan SkeletonExporter::getSkinAnimationTimespan( FbxAnimStack* animStack, FbxSkin *skin ){
    
    int lClusterCount = skin->GetClusterCount();
    
    
    FbxTimeSpan result;
    
    FbxTimeSpan AnimTimeSpan;
    
    for( int lClusterIndex = 0; lClusterIndex != lClusterCount; ++lClusterIndex )
    {
        FbxCluster* lCluster = skin->GetCluster( lClusterIndex );
        FbxNode* node = lCluster->GetLink();
        
        bool isAnimated = node->GetAnimationInterval( AnimTimeSpan, animStack );
        
        if( isAnimated )
            result.UnionAssignment( AnimTimeSpan );
    }
    
    return AnimTimeSpan;
    

}


AWDSkeletonAnimation* SkeletonExporter::exportAnimation( FbxAnimStack* animStack, FbxSkin *skin ){
    
    
    
    FbxTimeSpan AnimTimeSpan = getSkinAnimationTimespan(animStack, skin );
    //bool isAnimated = firstNode->GetAnimationInterval( AnimTimeSpan, animStack );
    
    FBXSDK_printf( "export Animation \n" );
    // no animation
    if ( AnimTimeSpan.GetDuration() <= 0 )
    {
        return NULL;
    }
    
    
    FbxString name = animStack->GetName();
    
    
    FBXSDK_printf( "export Animation %s \n", name.Buffer() );
    FBXSDK_printf( "   start %lli \n", AnimTimeSpan.GetStart().GetFrameCount() );
    FBXSDK_printf( "   start %lli \n", AnimTimeSpan.GetStop().GetFrameCount() );
    
    
    FbxLongLong startFrame = AnimTimeSpan.GetStart().GetFrameCount();
    FbxLongLong stopFrame  = AnimTimeSpan.GetStop().GetFrameCount();
    
    //const char *name, awd_uint16 name_len, int start_frame, int end_frame, int skip_frame, bool stitch_final, const char * sourceID, bool loop, bool useTransforms
    AWDSkeletonAnimation* animation = new AWDSkeletonAnimation(
                                                               name,
                                                               static_cast<unsigned short>( strlen( name ) ),
                                                               startFrame,
                                                               stopFrame,
                                                               false, false, "", false, false
                                                               );
    
    // export frames from start to stop
    // ------------
    
    FbxTime frameTime;
    
    for (FbxLongLong frame = startFrame; frame <= stopFrame; frame++) {
        frameTime.SetFrame( frame );
        AWDSkeletonPose* pose = CreateSkeletonPoseAtTime( frameTime, skin );
        mContext->GetAwd()->add_skeleton_pose( pose );
        animation->set_next_frame_pose( pose, 16 );
    }
    
    
    
    mContext->GetAwd()->add_skeleton_anim( animation );
    
    return animation;
}



AWDAnimationSet* SkeletonExporter::exportAnimationSet( FbxSkin *skin ){
    
    
    FbxScene *Scene = mContext->GetFbxScene();
    int AnimStackCount = Scene->GetSrcObjectCount<FbxAnimStack>();
    
    
     //name_len, animtype, sourcePreID, sourcePreID_len, AWDBlockList * sourcClipList
    AWDAnimationSet* animationSet = new AWDAnimationSet( "", 0, ANIMTYPESKELETON, NULL, 0, NULL );
    
    
    AWDBlockList *animations = new AWDBlockList();
    
    for( int AnimStackIndex = 0; AnimStackIndex < AnimStackCount; AnimStackIndex++ )
    {
        FbxAnimStack* CurAnimStack = Scene->GetSrcObject<FbxAnimStack>(AnimStackIndex);
        
        AWDSkeletonAnimation* animationBlock = exportAnimation(CurAnimStack, skin );
        
        if( animationBlock ){
            animations->append( animationBlock );
        }
        
    }
    
    animationSet->set_animationClipNodes( animations );
    
    mContext->GetAwd()->add_amin_set_block( animationSet );
    
    return animationSet;
}

    
void SkeletonExporter::exportAnimator( FbxSkin *skin, FbxMesh* mesh ){
    
    AWDSkeleton* skeleton = exportSkeleton( skin );
    AWDAnimationSet* animSet = exportAnimationSet( skin );
    
    FbxString meshName = mesh->GetName();
    meshName += "_animator";
    
    AWDAnimator* animator = new AWDAnimator( meshName, static_cast<unsigned short>( strlen( meshName ) ), animSet, ANIMTYPESKELETON );
    animator->set_skeleton( skeleton );
    
    mContext->add_animator( animator, mesh );
    
}

void SkeletonExporter::doExport(FbxObject* pObj){
    
    FbxNode *lNode = (FbxNode*) pObj;
    
    FbxSkeleton *skeleton = lNode->GetSkeleton();
    
    
    AWDSkeletonJoint *joint = new AWDSkeletonJoint( NULL, 0, NULL, 0 );
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
