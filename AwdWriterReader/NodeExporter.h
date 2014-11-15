//
//  NodeExporter.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 04/11/2014.
//
//

#ifndef __FbxAwdExporter__NodeExporter__
#define __FbxAwdExporter__NodeExporter__

#include <awd/awd.h>
#include <fbxsdk.h>

#include "ExportContext.h"
//This class is a custom writer.
//The writer provide you the ability to write out node hierarchy to a custom file format.


class NodeExporter
{
public:
    
    void setup( ExportContext *context );
    void release();
    
    virtual bool isHandleObject( FbxObject* ) = 0;
   
    virtual void doExport( FbxObject* ) = 0;
    
protected:
    
    ExportContext   *mContext;
};

//
// common utilities convert FBX data to AWD data
//
//
void CopyNodeTransform( FbxNode* pNode, AWDSceneBlock* sceneBlock );


#endif /* defined(__FbxAwdExporter__NodeExporter__) */
