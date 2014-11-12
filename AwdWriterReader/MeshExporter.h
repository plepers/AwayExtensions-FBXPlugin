//
//  MeshExporter.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 05/11/2014.
//
//

#ifndef __FbxAwdExporter__MeshExporter__
#define __FbxAwdExporter__MeshExporter__

#include "NodeExporter.h"


class MeshExporter : public NodeExporter
{
public:
    virtual bool isHandleObject( FbxObject* );
    virtual void doExport( FbxObject* );
};


#endif /* defined(__FbxAwdExporter__MeshExporter__) */
