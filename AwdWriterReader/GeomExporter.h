//
//  GeomExporter.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 08/11/2014.
//
//

#ifndef __FbxAwdExporter__GeomExporter__
#define __FbxAwdExporter__GeomExporter__


#include "NodeExporter.h"


class GeomExporter : public NodeExporter
{
public:
    virtual bool isHandleObject( FbxObject* );
    virtual void doExport( FbxObject* );
    
private:
    
    // For every material, record the offsets in every VBO and triangle counts
    struct SubMesh
    {
        SubMesh() : IndexOffset(0), TriangleCount(0) {}
        
        int IndexOffset;
        int TriangleCount;
    };
    
    FbxArray<SubMesh*> mSubMeshes;
    bool mHasNormal;
    bool mHasUV;
    bool mAllByControlPoint;
};


#endif /* defined(__FbxAwdExporter__GeomExporter__) */
