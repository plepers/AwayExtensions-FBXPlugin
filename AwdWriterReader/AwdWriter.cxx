/****************************************************************************************

   Copyright (C) 2014 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "AwdWriter.h"
#include "AwdWriterReader.h"
#include "Settings.h"

#include <ContainerExporter.h>
#include <MeshExporter.h>

#include <awd/awd.h>




/*
 * The default ExportProvider factory
 *
 */
ExporterProvider* CreateExporterProvider(){
    
    ContainerExporter* pDefault = new ContainerExporter();
    
    ExporterProvider* provider = new ExporterProvider( pDefault );
    
    // add exporter for each fbx nodes types here
    provider->addExporter( new MeshExporter() );
    
    return provider;
}




/*
 * AwdWriter
 * ------
 * implement FbxWriter, entry point for export
 */

AwdWriter::AwdWriter(FbxManager &pManager, int pID):
FbxWriter(pManager, pID, FbxStatusGlobal::GetRef()),
mFilePointer(NULL),
mManager(&pManager)
{
	
}

AwdWriter::~AwdWriter()
{
    FileClose();
}

// Create a file stream with pFileName
bool AwdWriter::FileCreate(char* pFileName)
{
    if(mFilePointer != NULL)
    {
        FileClose();
    }
    FBXSDK_fopen(mFilePointer,pFileName,"w");
    if(mFilePointer == NULL)
    {
        return false;
    }
    return true;
}

// Close the file stream
bool AwdWriter::FileClose()
{
    if(mFilePointer != NULL)
    {
        fclose(mFilePointer);
        return true;
    }
    return false;
}

// Check whether the file stream is open.
bool AwdWriter::IsFileOpen()
{
    if(mFilePointer != NULL)
        return true;
    return false;
}

// Get the file stream options
void AwdWriter::GetWriteOptions()
{
}

// Write file with stream options
bool AwdWriter::Write(FbxDocument* pDocument)
{
    if (!pDocument)
    {
        GetStatus().SetCode(FbxStatus::eFailure, "Invalid document handle");
        return false;
    }

    FbxScene* lScene = FbxCast<FbxScene>(pDocument);
    bool lIsAScene = (lScene != NULL);
    bool lResult = false;

    if(lIsAScene)
    {
        PreprocessScene(*lScene);
        FBXSDK_printf("I'm in my own writer\n");

        ExportScene(lScene);

        PostprocessScene(*lScene);
        lResult = true;
    }
    return lResult;
}



// Pre-process the scene before write it out
bool AwdWriter::PreprocessScene(FbxScene& /*pScene*/)
{
    FbxIOSettings* s = GetIOSettings();
    
//    FbxString dir( "/Users/plepers/work/workspaces/c/totos.xml" );
//    FbxString name( "name.xml" );
//    FbxString props( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|Test" );
//    FBXSDK_printf("%s\n", props.Buffer() );
//    s->WriteXMLFile( dir );
    
    mAwd = AwdSettings::createAwd( s );
    mExporters = CreateExporterProvider();
    mBlocksMap = new BlocksMap();
    
    FBXSDK_printf("I'm in pre-process\n");
    return true;
}



// Post-process the scene after write it out
bool AwdWriter::PostprocessScene(FbxScene& /*pScene*/)
{
    int fd = fileno( mFilePointer );
    mAwd->flush( fd );
    mExporters = NULL;
    
    FBXSDK_printf("I'm in post process\n");
    return true;
}



// Write out Node Hierarchy recursively
void AwdWriter::ExportScene(FbxScene* pScene )
{
    
    // export global scene settings here
    
    ExportNodeAndChildren( pScene->GetRootNode() );
    
}


// export node descendants then node
// return true if one or more node has been exported
//
bool AwdWriter::ExportNodeAndChildren(FbxNode* pNode)
{
    bool exported = false;
    
    FbxNode* lChildNode;
    
    int lNodeChildCount = pNode->GetChildCount ();
    while (lNodeChildCount > 0)
    {
        lNodeChildCount--;
        lChildNode = pNode->GetChild (lNodeChildCount);
        
        if( ExportNodeAndChildren(lChildNode) ) {
            exported = true;
        }
    }
    
    // force export of this node if exported is true
    // since it must be parent of exported children
    ExportNode( pNode, exported );
    
    return exported;

}


/*
 * Retreive an exporter matching node type
 * and execute it
 * if `force` is false and no exporter is found
 * skip the node
 */
bool AwdWriter::ExportNode(FbxNode* pNode, bool force )
{
    
    FBXSDK_printf("ExportNode %s\n", pNode->GetName() );
    
//    FbxNodeAttribute* attr = pNode->GetNodeAttribute();
//    if( attr ){
//        FBXSDK_printf("  attr %s\n", attr->GetName() );
//        
//        attr = attr->GetNodeAttribute();
//        if( attr )
//            FBXSDK_printf("      attr %s\n", attr->GetName() );
//
//    }
    
    
    bool exported = force;
    
    NodeExporter* exporter = NULL;
    
    if(pNode->GetNodeAttribute() != NULL)
    {
        exporter = mExporters->findExporter( pNode );
    }
    
    if( exporter == NULL && force ){
        exporter = mExporters->getDefaultExporter();
    }
    
    if( exporter ){
        exporter->setup(mAwd, mManager, mBlocksMap);
        exporter->doExport( pNode );
        exporter->release();
        exported = true;
    }
    
    
    return exported;
    
}




// ================
// ExporterProvider


ExporterLinkedItem::ExporterLinkedItem(NodeExporter *exporter )
{
    mExporter = exporter;
    mNext = NULL;
}

ExporterLinkedItem::~ExporterLinkedItem()
{
    mExporter = NULL;
}


ExporterLinkedItem* ExporterLinkedItem::next()
{
    return mNext;
}

void ExporterLinkedItem::setNext( ExporterLinkedItem* item )
{
    if( mNext ) {
        item->setNext( mNext );
    }
    mNext = item;
}

NodeExporter* ExporterLinkedItem::getExporter()
{
    return mExporter;
}






ExporterProvider::ExporterProvider( NodeExporter* pDefault )
{
    mDefault = pDefault;
    mHead = NULL;
    mTail = NULL;
}

void ExporterProvider::addExporter(NodeExporter *exporter)
{
    ExporterLinkedItem *item = new ExporterLinkedItem( exporter );
    if( mTail ) {
        mTail->setNext(item);
    } else {
        mHead = mTail = item;
    }
}

NodeExporter* ExporterProvider::findExporter( FbxObject* pObj )
{
    ExporterLinkedItem *item = mHead;
    
    while( item ) {
        
        if( item->getExporter()->isHandleObject( pObj ) ){
            return item->getExporter();
        }
        
        item = item->next();
    }
    
    // no suitable exporter found
    return NULL;
}



NodeExporter* ExporterProvider::getDefaultExporter()
{
    return mDefault;
}


