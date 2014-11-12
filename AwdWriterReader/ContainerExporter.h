//
//  ContainerExporter.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 04/11/2014.
//
//

#ifndef __FbxAwdExporter__ContainerExporter__
#define __FbxAwdExporter__ContainerExporter__

#include "NodeExporter.h"


class ContainerExporter : public NodeExporter
{
    virtual bool isHandleObject( FbxObject* );
    
    virtual void doExport( FbxObject* );
};


#endif /* defined(__FbxAwdExporter__ContainerExporter__) */
