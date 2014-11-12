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
#include <awd/awd.h>
#include <NodeExporter.h>
#include <BlocksMap.h>





//
// ExporterProvider
// Manage a list of node exporter
// retreive the correct NodeExporter for
// each nodes
//

class ExporterLinkedItem
{
public:
    ExporterLinkedItem( NodeExporter* exporter );
    ~ExporterLinkedItem();
    
    ExporterLinkedItem* next();
    void setNext( ExporterLinkedItem*  );
    
    NodeExporter* getExporter();
    
    
private:
    
    NodeExporter* mExporter;
    ExporterLinkedItem* mNext;
    
};




class ExporterProvider
{
public:
    ExporterProvider( NodeExporter* pDefault );
    ~ExporterProvider();
    
    void addExporter( NodeExporter* );
    
    NodeExporter* findExporter( FbxObject* pObj );
    NodeExporter* getDefaultExporter();
    
private:
    ExporterLinkedItem 	*mHead;
    ExporterLinkedItem 	*mTail;
    NodeExporter		*mDefault;
};







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
    virtual bool ExportNode(FbxNode* pNode, bool force=false );

private:
    FILE				*mFilePointer;
    FbxManager			*mManager;
    AWD					*mAwd;
    ExporterProvider	*mExporters;
    BlocksMap           *mBlocksMap;
};



#endif
