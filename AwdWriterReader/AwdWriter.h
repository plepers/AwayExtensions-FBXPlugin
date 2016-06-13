/****************************************************************************************

   Copyright (C) 2014 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef AWD_WRITER_H
#define AWD_WRITER_H

#include <fbxsdk.h>
#include "awd.h"
#include <ExportContext.h>
#include "NodeExporter.h"




//This class is a custom writer.
//The writer provide you the ability to write out node hierarchy to a custom file format.
class AwdWriter : public FbxWriter
{
public:
    AwdWriter(FbxManager &pManager, int pID);

    //VERY important to put the file close in the destructor
    virtual ~AwdWriter();

    virtual bool FileCreate(char* pFileName) ;
    virtual bool FileClose() ;
    virtual bool IsFileOpen();
    virtual void GetWriteOptions() ;
    virtual bool Write(FbxDocument* pDocument);

    virtual bool PreprocessScene(FbxScene &pScene);
    virtual bool PostprocessScene(FbxScene &pScene);
    virtual void ExportScene(FbxScene* pScene);
    
    virtual bool ExportNodeAndChildren(FbxNode* pNode);
    virtual bool ExportChildren(FbxNode* pNode);
    virtual bool ExportNode(FbxNode* pNode, bool force=false );

private:
    
    ExporterProvider*   CreateExporterProvider();
    
    FILE				*mFilePointer;
    FbxManager			*mManager;
    ExportContext       *mContext;
    ExporterProvider	*mExporters;
    
};



#endif
