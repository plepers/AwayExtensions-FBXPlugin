//
//  ExportContext.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 15/11/2014.
//
//

#include "ExportContext.h"


#include "Settings.h"




ExportContext::ExportContext( FbxIOSettings* pSettings, FbxScene* pScene, FbxManager* pFbxMngr )
{
    mSettings = new Settings( pSettings );
    mManager  = pFbxMngr;
    mScene    = pScene;
    
    mBlocksMap    = new BlocksMap<AWDBlock>();
    mAnimatorsMap = new BlocksMap<AWDAnimator>();
    
    BlockSettings *lBlockSettings = new BlockSettings( false, false, false, false, 1.0 );
    mSettings->setupBlockSettings(lBlockSettings);
    
    mAwd = new AWD(
        mSettings->get_compression(),   
        0,                              
        NULL,                           
        mSettings->get_split_by_root(), 
        lBlockSettings,
        mSettings->get_export_empty()   
    );

}


ExportContext::~ExportContext()
{
    delete mAwd;
    delete mBlocksMap;
}


void ExportContext::add_texture(AWDBitmapTexture * block, FbxObject *pObj ){
    mAwd->add_texture( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_shadow(AWDShadowMethod * block, FbxObject *pObj ){
    mAwd->add_shadow( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_cube_texture(AWDCubeTexture * block, FbxObject *pObj ){
    mAwd->add_cube_texture( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_material(AWDMaterial * block, FbxObject *pObj ){
    mAwd->add_material( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_mesh_data(AWDBlock * block, FbxObject *pObj ){
    mAwd->add_mesh_data( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_skeleton(AWDSkeleton * block, FbxObject *pObj ){
    mAwd->add_skeleton( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_skeleton_pose(AWDSkeletonPose * block, FbxObject *pObj ){
    mAwd->add_skeleton_pose( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_skeleton_anim(AWDSkeletonAnimation * block, FbxObject *pObj ){
    mAwd->add_skeleton_anim( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_animator(AWDAnimator * block, FbxObject *pObj ){
    mAwd->add_animator( block );
    mAnimatorsMap->Set( pObj, block );
}
void ExportContext::add_uv_anim(AWDUVAnimation * block, FbxObject *pObj ){
    mAwd->add_uv_anim( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_scene_block(AWDSceneBlock * block, FbxObject *pObj ){
    if( block->get_parent() == NULL )
        mAwd->add_scene_block( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_light_picker_block(AWDLightPicker * block, FbxObject *pObj ){
    mAwd->add_light_picker_block( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_prim_block(AWDPrimitive * block, FbxObject *pObj ){
    mAwd->add_prim_block( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_amin_set_block(AWDAnimationSet * block, FbxObject *pObj ){
    mAwd->add_amin_set_block( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_vertex_anim_block(AWDVertexAnimation * block, FbxObject *pObj ){
    mAwd->add_vertex_anim_block( block );
    mBlocksMap->Set( pObj, block );
}
void ExportContext::add_effect_method_block(AWDEffectMethod * block, FbxObject *pObj ){
    mAwd->add_effect_method_block( block );
    mBlocksMap->Set( pObj, block );
}