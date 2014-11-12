
#include "AwdWriterReader.h"
#include "Settings.h"

#define AWD_OPTION_GROUP EXP_ADV_OPT_GRP

AWD* AwdSettings::createAwd( FbxIOSettings *pIOS ){

    BlockSettings * bs = new BlockSettings(
        AwdSettings::get_wide_matrix(pIOS),
        AwdSettings::get_wide_props(pIOS),
        AwdSettings::get_wide_attribs(pIOS),
        AwdSettings::get_wide_geoms(pIOS),
        AwdSettings::get_scale(pIOS)
    );

    AWD* awd = new AWD(
        AwdSettings::get_compression(pIOS),     // AWD_compression compression,
        0,                                      // awd_uint16 flags,
        NULL,                            		// char *outPathName,
        AwdSettings::get_split_by_root(pIOS),   // bool splitByRootObjs,
        bs,                                     // BlockSettings * thisBlockSettings,
        AwdSettings::get_export_empty(pIOS)     // bool exportEmtpyContainers
    );

    return awd;
}

void AwdSettings::setupBlockSettings( FbxIOSettings *pIOS, BlockSettings * bs){
    bs->set_wide_matrix(        AwdSettings::get_wide_matrix(pIOS) );
    bs->set_wide_props(         AwdSettings::get_wide_props(pIOS) );
    bs->set_wide_attributes(    AwdSettings::get_wide_attribs(pIOS) );
    bs->set_wide_geom(          AwdSettings::get_wide_geoms(pIOS) );
    bs->set_scale(              AwdSettings::get_scale(pIOS) );
}

void AwdSettings::FillFbxIOSettings(FbxIOSettings& pIOS ){


    // Here you can write your own FbxIOSettings and parse them.
    FbxProperty FBXExtentionsSDKGroup = pIOS.GetProperty(AWD_OPTION_GROUP);
    if( !FBXExtentionsSDKGroup.IsValid() ) return;

    FbxProperty IOPluginGroup = pIOS.AddPropertyGroup(FBXExtentionsSDKGroup, PLUGIN_NAME, FbxStringDT, PLUGIN_NAME);

    if( IOPluginGroup.IsValid() )
    {

        bool precision_mtx      = false;
        bool precision_geo      = false;
        bool precision_props    = false;
        bool precision_attr     = false;
        float scale             = 1.0;
//        AWD_compression comp    = UNCOMPRESSED;

        bool export_empty       = true;
        bool split_by_root      = false;


        pIOS.AddProperty(IOPluginGroup, WIDE_MATRIX ,       FbxBoolDT,  "Wide Matrix",  &precision_mtx );
        pIOS.AddProperty(IOPluginGroup, WIDE_GEOMS  ,       FbxBoolDT,  "Wide Geoms",   &precision_geo );
        pIOS.AddProperty(IOPluginGroup, WIDE_PROPS  ,       FbxBoolDT,  "Wide Props",   &precision_props );
        pIOS.AddProperty(IOPluginGroup, WIDE_ATTIBS ,       FbxBoolDT,  "Wide Attribs", &precision_attr );
        pIOS.AddProperty(IOPluginGroup, EXPORT_SCALE,       FbxFloatDT, "Scene Scale",  &scale );

        pIOS.AddProperty(IOPluginGroup, EXPORT_EMPTY ,      FbxBoolDT,  "export empty containers", &export_empty );
        pIOS.AddProperty(IOPluginGroup, SPLIT_BY_ROOT ,     FbxBoolDT,  "split by root", &split_by_root );

        FbxProperty compression = pIOS.AddProperty(IOPluginGroup, AWD_COMPRESSION,    FbxEnumDT, "Compression",  AWD_COMPRESSION );

        compression.AddEnumValue( "uncompressed" );
        compression.AddEnumValue( "deflate" );
        compression.AddEnumValue( "lzma" );

    }

}


FbxProperty AwdSettings::getSettingsGroup(FbxIOSettings* pIOS){
    FbxProperty group = pIOS->GetProperty( AWD_OPTION_GROUP "|" PLUGIN_NAME );
    return group;
}

bool AwdSettings::get_wide_matrix(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" WIDE_MATRIX, false );
}

bool AwdSettings::get_wide_geoms(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" WIDE_GEOMS, false );
}

bool AwdSettings::get_wide_props(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" WIDE_PROPS, false );
}

bool AwdSettings::get_wide_attribs(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" WIDE_ATTIBS, false );
}

bool AwdSettings::get_export_empty(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" EXPORT_EMPTY, false );
}

bool AwdSettings::get_split_by_root(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" SPLIT_BY_ROOT, false );
}

double AwdSettings::get_scale(FbxIOSettings* pIOS){
    return pIOS->GetDoubleProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" EXPORT_SCALE, 1.0 );
}


void AwdSettings::set_wide_matrix(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" WIDE_MATRIX, value );
}

void AwdSettings::set_wide_geoms(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" WIDE_GEOMS, value );
}

void AwdSettings::set_wide_props(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" WIDE_PROPS, value );
}

void AwdSettings::set_wide_attribs(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" WIDE_ATTIBS, value );
}

void AwdSettings::set_export_empty(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" EXPORT_EMPTY, value );
}

void AwdSettings::set_split_by_root(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" SPLIT_BY_ROOT, value );
}

void AwdSettings::set_scale(FbxIOSettings* pIOS, double value){
    pIOS->SetDoubleProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" EXPORT_SCALE, value );
}


AWD_compression AwdSettings::get_compression        (FbxIOSettings* pIOS){
    FbxString str = pIOS->GetEnumProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" AWD_COMPRESSION, "uncompressed");
    if( str == "uncompressed" )
        return UNCOMPRESSED;
    if( str == "deflate" )
        return DEFLATE;
    if( str == "lzma" )
        return LZMA;

    return UNCOMPRESSED;
}

void AwdSettings::set_compression        (FbxIOSettings* pIOS, AWD_compression value){
    FbxString str;
    if( value == UNCOMPRESSED )
        str = "uncompressed";
    if( value == DEFLATE )
        str = "deflate";
    if( value == LZMA )
        str = "lzma";

    pIOS->SetEnumProp( AWD_OPTION_GROUP "|" PLUGIN_NAME "|" AWD_COMPRESSION, str );
}

