//
//  utils.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 16/11/2014.
//
//

#ifndef __FbxAwdExporter__utils__
#define __FbxAwdExporter__utils__

#include <fbxsdk.h>
#include <awd/awd.h>

namespace AwdUtils {
    
    void CopyNodeTransform( FbxNode* pNode, AWDSceneBlock* sceneBlock );
    
    bool isMatrix2dIdentity( awd_float64 *mtx );
}

#endif /* defined(__FbxAwdExporter__utils__) */
