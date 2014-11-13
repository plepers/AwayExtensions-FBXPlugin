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
    
    struct SubMeshData
    {
        SubMeshData() :
        	numVertices(0),
        	indices(NULL),
        	vertices(NULL),
        	normals(NULL),
        	uvs(NULL) {}
        
        unsigned int 	numVertices;
        unsigned int 	*indices;
        
        awd_float64 	*vertices;
        awd_float64		*normals;
        awd_float64		*uvs;
    };
    
    struct SubMesh
    {
        SubMesh() : IndexOffset(0), TriangleCount(0), data(NULL) {}
        
        int IndexOffset;
        int TriangleCount;
        SubMeshData *data;
        
    };
    
    FbxArray<SubMesh*> mSubMeshes;
    
    bool mHasNormal;
    bool mHasUV;
    bool mAllByControlPoint;
};


#endif /* defined(__FbxAwdExporter__GeomExporter__) */
