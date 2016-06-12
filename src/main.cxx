/****************************************************************************************



****************************************************************************************/


#include <fbxsdk.h>

#include "Common.h"
#include "ExporterConfig.h"
#include "Settings.h"

#include <tclap/CmdLine.h>
#include <tclap/ArgException.h>

#define G_INCLUDES_UV  "uv"
#define G_INCLUDES_UV2 "uv2"
#define G_INCLUDES_NRM "normal"
#define G_INCLUDES_TGT "tangent"
#define G_INCLUDES_BNR "binorm"
#define G_INCLUDES_CLR "color"
#define G_INCLUDES_SKN "skin"



int main(int argc, char** argv)
{
    
    std::string input;
    std::string output;
    std::string compression;
    AWD_compression compressionEnum;
    
    bool attrPrecision  = false;
    bool geomPrecision  = false;
    bool matrPrecision  = false;
    bool propPrecision  = false;
    
    bool exportUv       = true;
    bool exportUv2      = true;
    bool exportNormal   = true;
    bool exportTangent  = true;
    bool exportBinorm   = true;
    bool exportColor    = true;
    bool exportSkin     = true;
    
    bool tootleOptim    = true;
    
    bool embedTextures;
    bool exportEmpty;
    double sceneScale;
    
    
    
    // Parse cmdlne args using tclap
    try {
        

        TCLAP::CmdLine cmd("fbx2awd converter", ' ', FbxAwdExporter_VERSION_STR);

        
        TCLAP::UnlabeledValueArg<std::string> inputArg( "input", "file's path to convert", true, "", "input" );
        
        TCLAP::ValueArg<std::string> outputArg("o","output","output file's path", false, "", "output AWD file's path");
        TCLAP::ValueArg<float> scaleArg("s","scale","scale", false, 1.0, "scale");
        
        cmd.add( inputArg );
        cmd.add( outputArg );
        cmd.add( scaleArg );
        
        
        // precision options
        //
        TCLAP::SwitchArg precisionSwitch("W","precision","export all datas with double precision", cmd, false);
        TCLAP::SwitchArg wideAttrSwitch("","wa","export attributes with double precision", cmd, false);
        TCLAP::SwitchArg wideGeomSwitch("","wg","export geometries with double precision", cmd, false);
        TCLAP::SwitchArg wideMatrSwitch("","wm","export matrices with double precision", cmd, false);
        TCLAP::SwitchArg widePropSwitch("","wp","export properties with double precision", cmd, false);
        
        
        // misc options
        //
        TCLAP::SwitchArg embedTexSwitch("t","textures","embed textures", cmd, false);
        TCLAP::SwitchArg exportEmptySwitch("e","empty","export empty containers", cmd, true);
        
        
        // geom streams options
        //
        
        std::vector<std::string> vdataEnums;
        vdataEnums.push_back( "none" );
        vdataEnums.push_back( G_INCLUDES_UV );
        vdataEnums.push_back( G_INCLUDES_UV2 );
        vdataEnums.push_back( G_INCLUDES_NRM );
        vdataEnums.push_back( G_INCLUDES_TGT );
        vdataEnums.push_back( G_INCLUDES_CLR );
        vdataEnums.push_back( G_INCLUDES_SKN );
        TCLAP::ValuesConstraint<std::string> allowedVdata( vdataEnums );
        
        TCLAP::MultiArg<std::string> streamIncArg("g","geom","include given vertex data to geometry", false, &allowedVdata );
        TCLAP::MultiArg<std::string> streamExcArg("G","GEOM","exclude given vertex data from geometry", false, &allowedVdata );
        
        cmd.add( streamIncArg );
        cmd.add( streamExcArg );
        
        
        //compression options
        //
        std::vector<std::string> compEnums;
        compEnums.push_back("uncompressed");
        compEnums.push_back("lzma");
        compEnums.push_back("deflate");
        TCLAP::ValuesConstraint<std::string> allowedComps( compEnums );
        TCLAP::ValueArg<std::string> compressArg( "c","compression","compression type", false, "uncompressed", &allowedComps);
        
        // tootle options
        //
        
        TCLAP::SwitchArg notootleSwitch("","notootle","skip tootle optimisations", cmd, false);
        
        cmd.add( compressArg );
        
        
        cmd.parse( argc, argv );
        
        // Get the value parsed by each arg.
        input           = inputArg.getValue();
        output          = outputArg.getValue();
        compression     = compressArg.getValue();
        embedTextures   = embedTexSwitch.getValue();
        exportEmpty     = exportEmptySwitch.getValue();
        sceneScale      = scaleArg.getValue();
        
        attrPrecision   = precisionSwitch.getValue() || wideAttrSwitch.getValue();
        geomPrecision   = precisionSwitch.getValue() || wideGeomSwitch.getValue();
        matrPrecision   = precisionSwitch.getValue() || wideMatrSwitch.getValue();
        propPrecision   = precisionSwitch.getValue() || widePropSwitch.getValue();
        
        tootleOptim     = ! notootleSwitch.getValue();
        
        // stream list
        //
        std::vector<std::string> streamList;
        bool streamDefault = false;
        
        if( streamExcArg.isSet() && streamIncArg.isSet() ){
            
            throw( TCLAP::CmdLineParseException("-g and -G can't be both set.", "-g / -G" ) );
        }
        
        if( streamIncArg.isSet() )
        {
            streamDefault = false;
            streamList = streamIncArg.getValue();
        }
        else if( streamExcArg.isSet() )
        {
            streamDefault = true;
            streamList = streamExcArg.getValue();
        }
        
        if( streamExcArg.isSet() || streamIncArg.isSet() ){
            exportUv = exportUv2 = exportNormal  = exportTangent = exportBinorm = exportColor = streamDefault;
            
            for(std::vector<std::string>::iterator it = streamList.begin(); it != streamList.end(); ++it)
            {
                if( *it == G_INCLUDES_UV )
                {
                    exportUv = !streamDefault;
                }
                else if( *it == G_INCLUDES_UV2 )
                {
                    exportUv2 = !streamDefault;
                }
                else if( *it == G_INCLUDES_NRM )
                {
                    exportNormal = !streamDefault;
                }
                else if( *it == G_INCLUDES_TGT )
                {
                    exportTangent = !streamDefault;
                }
                else if( *it == G_INCLUDES_BNR )
                {
                    exportBinorm = !streamDefault;
                }
                else if( *it == G_INCLUDES_CLR )
                {
                    exportColor = !streamDefault;
                }
                else if( *it == G_INCLUDES_SKN )
                {
                    exportSkin = !streamDefault;
                }
            }
        }
        
//        FBXSDK_printf("Include in geom : \n");
//        if( exportUv )      FBXSDK_printf("   uv\n");
//        if( exportUv2 )     FBXSDK_printf("   uv2\n");
//        if( exportNormal )  FBXSDK_printf("   normals\n");
//        if( exportTangent ) FBXSDK_printf("   tangents\n");
//        if( exportBinorm )  FBXSDK_printf("   binorms\n");
//        if( exportColor )   FBXSDK_printf("   colors\n");
        
        
        if( compression == "uncompressed" ) {
            compressionEnum = UNCOMPRESSED;
        } else if( compression == "lzma" ){
            compressionEnum = LZMA;
        } else if( compression != "deflate" ){
            compressionEnum = DEFLATE;
        } else {
            throw("Unsupported compression (-c) argument, should be < uncompressed | lzma | deflate >");
        }
        
        
        
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
    
    
    

    
    FbxString lInputPath( input.c_str() );
    FbxString lOutputPath( output.c_str() );
    
    if( lInputPath.IsEmpty() ) {
        // This should already be handled by tclap
        FBXSDK_printf("Error: empty input file\n");
    }
    
    
    if( lOutputPath.IsEmpty() ) {
        const size_t lFileNameLength = strlen(lInputPath.Buffer());
        char* lNewFileName = new char[lFileNameLength+64];
        FBXSDK_strcpy(lNewFileName,lFileNameLength+64,lInputPath.Buffer());
        FBXSDK_strcpy(lNewFileName+lFileNameLength-3,61, FbxAwdExporter_PLUGIN_EXTENSION);
        lOutputPath = lNewFileName;
        delete[] lNewFileName;
    }

    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);


    bool lResult = LoadScene(lSdkManager, lScene, lInputPath.Buffer());
    if( lResult )
    {
        
        
        // Retrieve the writer ID according to the description of file format.
        int lFormat = lSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription(FbxAwdExporter_PLUGIN_DESCRIPTION);
        
        // Create an exporter.
        FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");
        
        // setup FbxIOSettings
        FbxIOSettings* ioSettings = lSdkManager->GetIOSettings();
        
        Settings *settings = new Settings( ioSettings );
        settings->set_compression(      compressionEnum);
        settings->set_wide_attribs(     attrPrecision);
        settings->set_wide_geoms(       geomPrecision);
        settings->set_wide_matrix(      matrPrecision);
        settings->set_wide_props(       propPrecision);
        settings->set_embed_textures(   embedTextures);
        settings->set_export_empty(     exportEmpty );
        settings->set_scale(            sceneScale );
        
        settings->set_export_geom_uv (  exportUv      );
        settings->set_export_geom_uv2(  exportUv2     );
        settings->set_export_geom_nrm(  exportNormal  );
        settings->set_export_geom_tgt(  exportTangent );
        settings->set_export_geom_bnr(  exportBinorm );
        settings->set_export_geom_clr(  exportColor   );
        settings->set_export_geom_skn(  exportSkin    );

        delete settings;
        
        // Initialize the exporter.
        lResult = lExporter->Initialize( lOutputPath, lFormat, ioSettings );
        if( !lResult )
        {
            FBXSDK_printf("%s:\tCall to FbxExporter::Initialize() failed.\n", lOutputPath.Buffer() );
            FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
        }
        else
        {
            // Export the scene.
            lResult = lExporter->Export(lScene);
            if( !lResult )
            {
                FBXSDK_printf("Call to FbxExporter::Export() failed.\n");
            }
        }
        
        // Destroy the exporter.
        lExporter->Destroy();
        
        
    }
    else
    {
        FBXSDK_printf("Call to LoadScene() failed.\n");
    }

    // Delete the FBX SDK manager. All the objects that have been allocated
    // using the FBX SDK manager and that haven't been explicitly destroyed
    // are automatically destroyed at the same time.
    DestroySdkObjects(lSdkManager, lResult);
    
    return 0;
}

