//
//  GeomData.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 11/06/2016.
//
//

#ifndef FbxAwdExporter_GeomData_h
#define FbxAwdExporter_GeomData_h


#include <awd/awd.h>


struct SubMeshData
{
    SubMeshData() :
    numIndices(0),
    numVertices(0),
    bonesPerVertex(0),
    indices(NULL),
    vertices(NULL),
    normals(NULL),
    tangent(NULL),
    binorm(NULL),
    colors(NULL),
    uvs(NULL),
    uvs2(NULL),
    skinWeights(NULL),
    skinIndices(NULL) {}
    
    unsigned int    numIndices;
    unsigned int 	numVertices;
    int             bonesPerVertex;
    
    unsigned int 	*indices;
    
    awd_float64     *vertices;
    awd_float64		*normals;
    awd_float64		*tangent;
    awd_float64		*binorm;
    awd_float64		*colors;
    awd_float64		*uvs;
    awd_float64		*uvs2;
    awd_float64		*skinWeights;
    awd_uint32      *skinIndices;
};

struct SubMesh
{
    SubMesh() : IndexOffset(0), TriangleCount(0), data(NULL) {}
    
    int IndexOffset;
    int TriangleCount;
    SubMeshData *data;
    
};


#endif
