//
//  ExportContext.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 15/11/2014.
//
//

#ifndef __FbxAwdExporter__ExportContext__
#define __FbxAwdExporter__ExportContext__

#include <fbxsdk.h>
#include "awd.h"
#include <BlocksMap.h>
#include "Settings.h"




class ExportContext
{
public:
    ExportContext( FbxIOSettings* pSettings, FbxManager* pFbxMngr );
    ~ExportContext();
    
    
    inline AWD* GetAwd() {
        return mAwd;
    }
    
    inline BlocksMap<AWDBlock>* GetBlocksMap() {
        return mBlocksMap;
    }
    
    inline FbxManager* GetFbxManager() {
        return mManager;
    }
    inline Settings* GetSettings() {
        return mSettings;
    }
    
    
    
    void add_texture            (AWDBitmapTexture *, FbxObject *pObj );
    void add_shadow             (AWDShadowMethod *,  FbxObject *pObj );
    void add_cube_texture       (AWDCubeTexture *, FbxObject *pObj );
    void add_material           (AWDMaterial *, FbxObject *pObj );
    void add_mesh_data          (AWDBlock *, FbxObject *pObj );
    void add_skeleton           (AWDSkeleton *, FbxObject *pObj );
    void add_skeleton_pose      (AWDSkeletonPose *, FbxObject *pObj );
    void add_skeleton_anim      (AWDSkeletonAnimation *, FbxObject *pObj );
    void add_animator           (AWDAnimator *, FbxObject *pObj );
    void add_uv_anim            (AWDUVAnimation *, FbxObject *pObj );
    void add_scene_block        (AWDSceneBlock *, FbxObject *pObj );
    void add_light_picker_block (AWDLightPicker *, FbxObject *pObj );
    void add_prim_block         (AWDPrimitive *, FbxObject *pObj );
    void add_amin_set_block     (AWDAnimationSet *, FbxObject *pObj );
    void add_vertex_anim_block  (AWDVertexAnimation *, FbxObject *pObj );
    void add_effect_method_block(AWDEffectMethod *, FbxObject *pObj );
    
    
private:
    FbxManager              *mManager;
    AWD                     *mAwd;
    BlocksMap<AWDBlock>     *mBlocksMap;
    Settings                *mSettings;
    
};

#endif /* defined(__FbxAwdExporter__ExportContext__) */
