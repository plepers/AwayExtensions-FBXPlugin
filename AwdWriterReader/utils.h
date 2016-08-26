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
#include "awd.h"

namespace AwdUtils {
    
    void CopyNodeName( FbxObject* , AWDNamedElement*  );
    void CopyNodeTransform( FbxNode* , AWDSceneBlock*  );
    void FbxMatrixTo4x3( FbxAMatrix* fbxMat, double* raw4x3 );
    
    awd_float64* mtx4x3_identity(awd_float64 *mtx);

    
    bool isMatrix2dIdentity( awd_float64 *mtx3x2 );
    
    
}

#endif /* defined(__FbxAwdExporter__utils__) */
