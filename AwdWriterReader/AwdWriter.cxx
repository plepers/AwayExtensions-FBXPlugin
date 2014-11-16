
#include <awd/awd.h>


#include "AwdWriter.h"
#include "AwdWriterReader.h"
#include "Settings.h"


#include <ContainerExporter.h>
#include <MeshExporter.h>

#include <awd/awd.h>




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
    FBXSDK_fopen(mFilePointer,pFileName,"wb");
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


/*
 * The default ExportProvider factory
 * register handlers for FbxNode subclasses
 *
 */
ExporterProvider* AwdWriter::CreateExporterProvider(){
    
    ContainerExporter* pDefault = new ContainerExporter();
    
    ExporterProvider* provider = new ExporterProvider( pDefault );
    
    // add exporter for each fbx nodes types here
    provider->addExporter( new MeshExporter() );
    
    return provider;
}




// Pre-process the scene before write it out
bool AwdWriter::PreprocessScene( FbxScene& pScene )
{
    FbxAxisSystem::MayaYUp.ConvertScene( &pScene );
    
    
    FbxIOSettings* s = GetIOSettings();
    
    mExporters = CreateExporterProvider();
    mContext = new ExportContext( s, mManager );
    
    return true;
}



// Post-process the scene after write it out
bool AwdWriter::PostprocessScene(FbxScene& /*pScene*/)
{
    int fd = fileno( mFilePointer );
    mContext->GetAwd()->flush( fd );
    
    delete mContext;
    delete mExporters;
    
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
    
    ExportNode( pNode, true );
    
    FbxNode* lChildNode;
    
    int lNodeChildCount = pNode->GetChildCount ();
    while (lNodeChildCount > 0)
    {
        lNodeChildCount--;
        lChildNode = pNode->GetChild( lNodeChildCount );
        
        if( ExportNodeAndChildren(lChildNode) ) {
            exported = true;
        }
    }
    
    return exported;
}


/*
 * Retreive an exporter matching node type
 * and execute it
 * if `force` is false and no exporter is found
 * skip the node
 */
bool AwdWriter::ExportNode( FbxNode* pNode, bool force )
{
    
    FBXSDK_printf("ExportNode %s\n", pNode->GetName() );
    
    
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
        exporter->setup( mContext, mExporters );
        exporter->doExport( pNode );
        exporter->release();
        exported = true;
    }
    
    
    return exported;
    
}



