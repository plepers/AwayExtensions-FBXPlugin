/****************************************************************************************

   Copyright (C) 2014 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "AwdWriter.h"
#include "AwdWriterReader.h"

#include <awd/awd.h>

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

        FbxNode* lRootNode = lScene->GetRootNode();
        PrintHierarchy(lRootNode);

        PostprocessScene(*lScene);
        lResult = true;
    }
    return lResult;
}

// Write out Node Hierarchy recursively
void AwdWriter::PrintHierarchy(FbxNode* pStartNode)
{
    FbxNode* lChildNode;
    const char* lParentName = pStartNode->GetName();
    for(int i = 0; i<pStartNode->GetChildCount(); i++)
    {
        lChildNode = pStartNode->GetChild(i);
        const char* lChildName = lChildNode->GetName();
        FBXSDK_fprintf(mFilePointer,"%s%s%s%s%s%s%s","\"",lChildName,"\"",", parent is ","\"",lParentName,"\"\n");
    }

    int lNodeChildCount = pStartNode->GetChildCount ();
    while (lNodeChildCount > 0)
    {
        lNodeChildCount--;
        lChildNode = pStartNode->GetChild (lNodeChildCount);
        PrintHierarchy(lChildNode);
    }
}

// Pre-process the scene before write it out
bool AwdWriter::PreprocessScene(FbxScene& /*pScene*/)
{
    FbxIOSettings* s = GetIOSettings();
    
    FbxString dir( "/Users/plepers/work/workspaces/c/toto.xml" );
    FbxString name( "name.xml" );
    FbxString props( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|Test" );
    
    FBXSDK_printf("%s\n", props.Buffer() );
    
    s->WriteXMLFile( dir );
    
    BlockSettings * awdBlockSettings=new BlockSettings(false,false,false,false, 1.0);
    
    AWD *awd = new AWD( UNCOMPRESSED, 0, "", false, awdBlockSettings, true );
    
    FBXSDK_printf("I'm in pre-process\n");
    return true;
}

// Post-process the scene after write it out
bool AwdWriter::PostprocessScene(FbxScene& /*pScene*/)
{
    FBXSDK_printf("I'm in post process\n");
    return true;
}
