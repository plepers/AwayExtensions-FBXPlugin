/****************************************************************************************

   Copyright (C) 2014 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "AwdWriterReader.h"
#include "Settings.h"

// Create your own writer.
// And your writer will get a pPluginID and pSubID.
FbxWriter* CreateAwdWriter(FbxManager& pManager, FbxExporter& pExporter, int /*pSubID*/, int pPluginID)
{
    // use FbxNew instead of new, since FBX will take charge its deletion
    FbxWriter* lWriter = FbxNew< AwdWriter >(pManager, pPluginID);
    lWriter->SetIOSettings(pExporter.GetIOSettings());
    return lWriter;
}

// Get extension, description or version info about AwdWriter
void* GetAwdWriterInfo(FbxWriter::EInfoRequest pRequest, int /*pId*/)
{
    static const char* sExt[] =
    {
        PLUGIN_EXTENSION,
        0
    };

    static const char* sDesc[] =
    {
        "Away3D AWD (*.AWD)",
        0
    };

    switch (pRequest)
    {
    case FbxWriter::eInfoExtension:
        return sExt;
    case FbxWriter::eInfoDescriptions:
        return sDesc;
    case FbxWriter::eInfoVersions:
        return 0;
    default:
        return 0;
    }
}

void FillAwdWriterIOSettings(FbxIOSettings& pIOS )
{
    AwdSettings::FillFbxIOSettings( pIOS );
}


// Creates a AwdReader in the Sdk Manager
FbxReader* CreateAwdReader(FbxManager& pManager, FbxImporter& pImporter, int /*pSubID*/, int pPluginID)
{
    // use FbxNew instead of new, since FBX will take charge its deletion

    FbxReader* lReader = FbxNew< AwdReader >(pManager, pPluginID);
    lReader->SetIOSettings(pImporter.GetIOSettings());
    return lReader;
}

// Get extension, description or version info about AwdReader
void *GetAwdReaderInfo(FbxReader::EInfoRequest pRequest, int pId)
{
    switch (pRequest)
    {
    case FbxReader::eInfoExtension:
        return GetAwdWriterInfo(FbxWriter::eInfoExtension, pId);
    case FbxReader::eInfoDescriptions:
        return GetAwdWriterInfo(FbxWriter::eInfoDescriptions, pId);
    default:
        return 0;
    }
}

void FillAwdReaderIOSettings(FbxIOSettings& /*pIOS*/)
{
    // Here you can write your own FbxIOSettings and parse them.
}
