//
//  TextureExporter.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 15/11/2014.
//
//

#ifndef __FbxAwdExporter__TextureExporter__
#define __FbxAwdExporter__TextureExporter__

#include "NodeExporter.h"


class TextureExporter : public NodeExporter
{
public:
    virtual bool isHandleObject( FbxObject* );
    virtual void doExport( FbxObject* );
};


#endif /* defined(__FbxAwdExporter__TextureExporter__) */
