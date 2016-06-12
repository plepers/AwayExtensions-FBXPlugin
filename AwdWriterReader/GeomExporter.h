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
#include "tootle.h"


// Experimental vertex color and binorms attribute ID
const AWD_mesh_str_type COLORS = static_cast<AWD_mesh_str_type>(11);
const AWD_mesh_str_type BINORMS = static_cast<AWD_mesh_str_type>(12);


class GeomExporter : public NodeExporter
{
public:
    
    void setup( ExportContext *context, ExporterProvider *ep );
    
    virtual bool isHandleObject( FbxObject* );
    virtual void doExport( FbxObject* );
    
    TootleSettings tootleSettings;
    
};


class Collapser
{
public:
    Collapser( unsigned int *pIndices, unsigned int pNumIndices, unsigned int pNumVertices );
    ~Collapser();
    
    void addStream( awd_float64 *data, unsigned int csize );
    void collapse();
    
private:
    
    typedef unsigned hash;
    
    struct Stream{
        Stream() :
        	data(NULL),
	        csize(0)
        {}
        
        awd_float64 	*data;
        unsigned int 	csize;
    };
    
    hash                hashVertex( char *vPtr, int len );
    void                remap();
    void                logStats();
    
    unsigned int 		mNumVertices;
    unsigned int 		mVertexSize;
    unsigned int 		mNumIndices;
    
    
    FbxArray<Stream*>	mStreams;
    unsigned int 		*mIndices;
    unsigned int 		*mRemapTable;
    
    
    
    void scanStream( unsigned int index, unsigned int comp );
};


#endif /* defined(__FbxAwdExporter__GeomExporter__) */
