//
//  MaterialExporter.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 11/11/2014.
//
//

#ifndef __FbxAwdExporter__MaterialExporter__
#define __FbxAwdExporter__MaterialExporter__

#include "NodeExporter.h"


class MaterialExporter : public NodeExporter
{
public:
    virtual bool isHandleObject( FbxObject* );
    virtual void doExport( FbxObject* );
};


#endif /* defined(__FbxAwdExporter__MaterialExporter__) */
