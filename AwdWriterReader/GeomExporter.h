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


class Collapser
{
public:
    Collapser( unsigned int *pIndices, unsigned int pNumIndices, unsigned int pNumVertices );
    ~Collapser();
    
    void addStream( awd_float64 *data, unsigned int csize );
    void collapse();
    
private:
    
    struct Stream{
        Stream() :
        	data(NULL),
	        csize(0)
        {}
        
        awd_float64 	*data;
        unsigned int 	csize;
    };
    
    unsigned int 		mNumVertices;
    unsigned int 		mNumIndices;
    
    
    FbxArray<Stream*>	mStreams;
    unsigned int 		*mIndices;
    unsigned int 		*mRemapTable;
    
    
    
    void scanStream( unsigned int index, unsigned int comp );
};


#endif /* defined(__FbxAwdExporter__GeomExporter__) */
