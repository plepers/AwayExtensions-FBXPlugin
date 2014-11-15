
#include "AwdWriterReader.h"
#include "Settings.h"


Settings::Settings( FbxIOSettings* pIOSettings )
{
    FillDefaultValues( pIOSettings );
    mIOSettings = pIOSettings;
}



void Settings::setupBlockSettings( BlockSettings * bs ){
    bs->set_wide_matrix(        get_wide_matrix() );
    bs->set_wide_props(         get_wide_props() );
    bs->set_wide_attributes(    get_wide_attribs() );
    bs->set_wide_geom(          get_wide_geoms() );
    bs->set_scale(              get_scale() );
}


void Settings::FillDefaultValues( FbxIOSettings *pIOS ){

    FbxProperty FBXExtentionsSDKGroup = pIOS->GetProperty(AWD_OPTION_GROUP);
    if( !FBXExtentionsSDKGroup.IsValid() ) return;

    FbxProperty IOPluginGroup = pIOS->AddPropertyGroup(FBXExtentionsSDKGroup, PLUGIN_NAME, FbxStringDT, PLUGIN_NAME);

    if( IOPluginGroup.IsValid() )
    {

        bool precision_mtx      = false;
        bool precision_geo      = false;
        bool precision_props    = false;
        bool precision_attr     = false;
        bool export_empty       = true;
        bool split_by_root      = false;
        float scale             = 1.0;

        
        if( ! wide_matrix_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, WIDE_MATRIX ,       FbxBoolDT,  "Wide Matrix",  &precision_mtx );
        }
        if( ! wide_geoms_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, WIDE_GEOMS  ,       FbxBoolDT,  "Wide Geoms",   &precision_geo );
        }
        if( ! wide_props_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, WIDE_PROPS  ,       FbxBoolDT,  "Wide Props",   &precision_props );
        }
        if( ! wide_attribs_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, WIDE_ATTIBS ,       FbxBoolDT,  "Wide Attribs", &precision_attr );
        }
        if( ! export_empty_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_EMPTY ,      FbxBoolDT,  "export empty containers", &export_empty );
        }
        if( ! split_by_root_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, SPLIT_BY_ROOT ,     FbxBoolDT,  "split by root", &split_by_root );
        }
        if( ! scale_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_SCALE,       FbxFloatDT, "Scene Scale",  &scale );
        }
        if( ! compression_property( pIOS ).IsValid() ) {
            FbxProperty compression = pIOS->AddProperty(IOPluginGroup, AWD_COMPRESSION,    FbxEnumDT, "Compression",  AWD_COMPRESSION );
            compression.AddEnumValue( "uncompressed" );
            compression.AddEnumValue( "deflate" );
            compression.AddEnumValue( "lzma" );
        }
        
        

    }

}


FbxProperty Settings::getSettingsGroup(){
    FbxProperty group = mIOSettings->GetProperty( AWD_OPTION_GROUP "|" PLUGIN_NAME );
    return group;
}

bool Settings::get_wide_matrix(){
    return mIOSettings->GetBoolProp( PROP_ID( WIDE_MATRIX ), false );
}

bool Settings::get_wide_geoms(){
    return mIOSettings->GetBoolProp( PROP_ID( WIDE_GEOMS ), false );
}

bool Settings::get_wide_props(){
    return mIOSettings->GetBoolProp( PROP_ID( WIDE_PROPS ), false );
}

bool Settings::get_wide_attribs(){
    return mIOSettings->GetBoolProp( PROP_ID( WIDE_ATTIBS ), false );
}

bool Settings::get_export_empty(){
    return mIOSettings->GetBoolProp( PROP_ID( EXPORT_EMPTY ), false );
}

bool Settings::get_split_by_root(){
    return mIOSettings->GetBoolProp( PROP_ID( SPLIT_BY_ROOT ), false );
}

double Settings::get_scale(){
    return mIOSettings->GetDoubleProp( PROP_ID( EXPORT_SCALE ), 1.0 );
}


void Settings::set_wide_matrix( bool value){
    mIOSettings->SetBoolProp( PROP_ID( WIDE_MATRIX ), value );
}

void Settings::set_wide_geoms( bool value){
    mIOSettings->SetBoolProp( PROP_ID( WIDE_GEOMS ), value );
}

void Settings::set_wide_props( bool value){
    mIOSettings->SetBoolProp( PROP_ID( WIDE_PROPS ), value );
}

void Settings::set_wide_attribs( bool value){
    mIOSettings->SetBoolProp( PROP_ID( WIDE_ATTIBS ), value );
}

void Settings::set_export_empty( bool value){
    mIOSettings->SetBoolProp( PROP_ID( EXPORT_EMPTY ), value );
}

void Settings::set_split_by_root( bool value){
    mIOSettings->SetBoolProp( PROP_ID( SPLIT_BY_ROOT ), value );
}

void Settings::set_scale( double value){
    mIOSettings->SetDoubleProp( PROP_ID( EXPORT_SCALE ), value );
}


FbxProperty Settings::wide_matrix_property(FbxIOSettings* pIOS) {
    return pIOS->GetProperty( PROP_ID( WIDE_MATRIX ) );
}

FbxProperty Settings::wide_geoms_property(FbxIOSettings* pIOS) {
    return pIOS->GetProperty( PROP_ID( WIDE_GEOMS ) );
}

FbxProperty Settings::wide_props_property(FbxIOSettings* pIOS) {
    return pIOS->GetProperty( PROP_ID( WIDE_PROPS ) );
}

FbxProperty Settings::wide_attribs_property(FbxIOSettings* pIOS) {
    return pIOS->GetProperty( PROP_ID( WIDE_ATTIBS ) );
}

FbxProperty Settings::export_empty_property(FbxIOSettings* pIOS) {
    return pIOS->GetProperty( PROP_ID( EXPORT_EMPTY ) );
}

FbxProperty Settings::split_by_root_property(FbxIOSettings* pIOS) {
    return pIOS->GetProperty( PROP_ID( SPLIT_BY_ROOT ) );
}

FbxProperty Settings::scale_property(FbxIOSettings* pIOS) {
    return pIOS->GetProperty( PROP_ID( EXPORT_SCALE ) );
}

FbxProperty Settings::compression_property(FbxIOSettings* pIOS) {
    return pIOS->GetProperty( PROP_ID( AWD_COMPRESSION ) );
}


AWD_compression Settings::get_compression(){
    FbxString str = mIOSettings->GetEnumProp( PROP_ID( AWD_COMPRESSION ), "uncompressed");
    if( str == "uncompressed" )
        return UNCOMPRESSED;
    if( str == "deflate" )
        return DEFLATE;
    if( str == "lzma" )
        return LZMA;

    return UNCOMPRESSED;
}

void Settings::set_compression        ( AWD_compression value){
    FbxString str;
    if( value == UNCOMPRESSED )
        str = "uncompressed";
    if( value == DEFLATE )
        str = "deflate";
    if( value == LZMA )
        str = "lzma";

    mIOSettings->SetEnumProp( PROP_ID( AWD_COMPRESSION ), str );
}

