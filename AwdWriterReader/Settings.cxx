
#include "AwdWriterReader.h"
#include "Settings.h"

void AwdSettings::setupAwd( FbxIOSettings *pIOS, AWD *awd ){
   
    
}

void setupBlockSettings( FbxIOSettings *pIOS, BlockSettings * bs){
    bs->set_wide_matrix(        AwdSettings::get_wide_matrix(pIOS) );
    bs->set_wide_props(         AwdSettings::get_wide_props(pIOS) );
    bs->set_wide_attributes(    AwdSettings::get_wide_attribs(pIOS) );
    bs->set_wide_geom(          AwdSettings::get_wide_geoms(pIOS) );
    bs->set_scale(              AwdSettings::get_scale(pIOS) );
}

void AwdSettings::FillFbxIOSettings(FbxIOSettings& pIOS ){
    
    
    // Here you can write your own FbxIOSettings and parse them.
    FbxProperty FBXExtentionsSDKGroup = pIOS.GetProperty(EXP_FBX_EXT_SDK_GRP);
    if( !FBXExtentionsSDKGroup.IsValid() ) return;
    
    FbxProperty IOPluginGroup = pIOS.AddPropertyGroup(FBXExtentionsSDKGroup, PLUGIN_NAME, FbxStringDT, PLUGIN_NAME);
    
    if( IOPluginGroup.IsValid() )
    {
        
        bool precision_mtx      = false;
        bool precision_geo      = false;
        bool precision_props    = false;
        bool precision_attr     = false;
        float scale             = 1.0;
        AWD_compression comp    = UNCOMPRESSED;
        
        
        pIOS.AddProperty(IOPluginGroup, WIDE_MATRIX ,       FbxBoolDT,  "Wide Matrix",  &precision_mtx );
        pIOS.AddProperty(IOPluginGroup, WIDE_GEOMS  ,       FbxBoolDT,  "Wide Geoms",   &precision_geo );
        pIOS.AddProperty(IOPluginGroup, WIDE_PROPS  ,       FbxBoolDT,  "Wide Props",   &precision_props );
        pIOS.AddProperty(IOPluginGroup, WIDE_ATTIBS ,       FbxBoolDT,  "Wide Attribs", &precision_attr );
        pIOS.AddProperty(IOPluginGroup, EXPORT_SCALE,       FbxFloatDT, "Scene Scale",  &scale );
        
        FbxProperty compression = pIOS.AddProperty(IOPluginGroup, AWD_COMPRESSION,    FbxEnumDT, "Compression",  AWD_COMPRESSION );
        
        compression.AddEnumValue( "uncompressed" );
        compression.AddEnumValue( "deflate" );
        compression.AddEnumValue( "lzma" );

    }
    
}


FbxProperty AwdSettings::getSettingsGroup(FbxIOSettings* pIOS){
    FbxProperty group = pIOS->GetProperty( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME );
    return group;
}

bool AwdSettings::get_wide_matrix(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" WIDE_MATRIX, false );
}

bool AwdSettings::get_wide_geoms(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" WIDE_GEOMS, false );
}

bool AwdSettings::get_wide_props(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" WIDE_PROPS, false );
}

bool AwdSettings::get_wide_attribs(FbxIOSettings* pIOS){
    return pIOS->GetBoolProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" WIDE_ATTIBS, false );
}

double AwdSettings::get_scale(FbxIOSettings* pIOS){
    return pIOS->GetDoubleProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" EXPORT_SCALE, 1.0 );
}


void AwdSettings::set_wide_matrix(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" WIDE_MATRIX, value );
}

void AwdSettings::set_wide_geoms(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" WIDE_GEOMS, value );
}

void AwdSettings::set_wide_props(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" WIDE_PROPS, value );
}

void AwdSettings::set_wide_attribs(FbxIOSettings* pIOS, bool value){
    pIOS->SetBoolProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" WIDE_ATTIBS, value );
}

void AwdSettings::set_scale(FbxIOSettings* pIOS, double value){
    pIOS->SetDoubleProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" EXPORT_SCALE, value );
}


AWD_compression AwdSettings::get_compression        (FbxIOSettings* pIOS){
    FbxString str = pIOS->GetEnumProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" AWD_COMPRESSION, "uncompressed");
    if( str == "uncompressed" )
        return UNCOMPRESSED;
    if( str == "deflate" )
        return DEFLATE;
    if( str == "lzma" )
        return LZMA;
}

void AwdSettings::set_compression        (FbxIOSettings* pIOS, AWD_compression value){
    FbxString str;
    if( value == UNCOMPRESSED )
        str = "uncompressed";
    if( value == DEFLATE )
        str = "deflate";
    if( value == LZMA )
        str = "lzma";
    
    pIOS->SetEnumProp( EXP_FBX_EXT_SDK_GRP "|" PLUGIN_NAME "|" AWD_COMPRESSION, str );
}

