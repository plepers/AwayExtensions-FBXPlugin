
#include "AwdWriterReader.h"
#include "Settings.h"

const bool  default_precision_mtx   = false;
const bool  default_precision_geo   = false;
const bool  default_precision_props = false;
const bool  default_precision_attr  = false;
const bool  default_export_empty    = true;
const bool  default_split_by_root   = false;
const float default_scale           = 1.0;

const bool  default_geom_uv         = true;
const bool  default_geom_uv2        = true;
const bool  default_geom_normal     = true;
const bool  default_geom_tangent    = true;
const bool  default_geom_binorm     = true;
const bool  default_geom_color      = false;
const bool  default_geom_skin       = true;


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

    FbxProperty IOPluginGroup = pIOS->AddPropertyGroup(FBXExtentionsSDKGroup, FbxAwdExporter_PLUGIN_NAME, FbxStringDT, FbxAwdExporter_PLUGIN_NAME);

    if( IOPluginGroup.IsValid() )
    {

        if( ! wide_matrix_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, WIDE_MATRIX ,       FbxBoolDT,  "Wide Matrix",  &default_precision_mtx );
        }
        if( ! wide_geoms_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, WIDE_GEOMS  ,       FbxBoolDT,  "Wide Geoms",   &default_precision_geo );
        }
        if( ! wide_props_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, WIDE_PROPS  ,       FbxBoolDT,  "Wide Props",   &default_precision_props );
        }
        if( ! wide_attribs_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, WIDE_ATTIBS ,       FbxBoolDT,  "Wide Attribs", &default_precision_attr );
        }
        if( ! export_empty_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_EMPTY ,      FbxBoolDT,  "export empty containers", &default_export_empty );
        }
        if( ! split_by_root_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, SPLIT_BY_ROOT ,     FbxBoolDT,  "split by root", &default_split_by_root );
        }
        if( ! scale_property( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_SCALE,       FbxFloatDT, "Scene Scale",  &default_scale );
        }
        if( ! geom_uv( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_GEOM_UV,     FbxBoolDT, "export UVs",           &default_geom_uv );
        }
        if( ! geom_uv2( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_GEOM_UV2,    FbxBoolDT, "export secondary UVs", &default_geom_uv2 );
        }
        if( ! geom_nrm( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_GEOM_NRM,    FbxBoolDT, "export normals",       &default_geom_normal );
        }
        if( ! geom_tgt( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_GEOM_TGT,    FbxBoolDT, "export tangents",      &default_geom_tangent );
        }
        if( ! geom_clr( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_GEOM_CLR,    FbxBoolDT, "export vertex colors", &default_geom_color );
        }
        if( ! geom_skn( pIOS ).IsValid() ) {
            pIOS->AddProperty(IOPluginGroup, EXPORT_GEOM_SKN,    FbxBoolDT, "export skin",          &default_geom_skin );
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
    FbxProperty group = mIOSettings->GetProperty( AWD_OPTION_GROUP "|" FbxAwdExporter_PLUGIN_NAME );
    return group;
}

bool Settings::get_wide_matrix(){
    return mIOSettings->GetBoolProp( PROP_ID( WIDE_MATRIX ), default_precision_mtx );
}

bool Settings::get_wide_geoms(){
    return mIOSettings->GetBoolProp( PROP_ID( WIDE_GEOMS ), default_precision_geo );
}

bool Settings::get_wide_props(){
    return mIOSettings->GetBoolProp( PROP_ID( WIDE_PROPS ), default_precision_props );
}

bool Settings::get_wide_attribs(){
    return mIOSettings->GetBoolProp( PROP_ID( WIDE_ATTIBS ), default_precision_attr );
}

bool Settings::get_export_empty(){
    return mIOSettings->GetBoolProp( PROP_ID( EXPORT_EMPTY ), default_export_empty );
}

bool Settings::get_split_by_root(){
    return mIOSettings->GetBoolProp( PROP_ID( SPLIT_BY_ROOT ), default_split_by_root );
}

double Settings::get_scale(){
    return mIOSettings->GetDoubleProp( PROP_ID( EXPORT_SCALE ), default_scale );
}

bool Settings::get_embed_textures(){
    return mIOSettings->GetBoolProp( EXP_EMBEDTEXTURE, false );
}

bool Settings::get_export_geom_uv  (){ return mIOSettings->GetBoolProp( PROP_ID( EXPORT_GEOM_UV  ), default_geom_uv      ); }
bool Settings::get_export_geom_uv2 (){ return mIOSettings->GetBoolProp( PROP_ID( EXPORT_GEOM_UV2 ), default_geom_uv2     ); }
bool Settings::get_export_geom_nrm (){ return mIOSettings->GetBoolProp( PROP_ID( EXPORT_GEOM_NRM ), default_geom_normal  ); }
bool Settings::get_export_geom_tgt (){ return mIOSettings->GetBoolProp( PROP_ID( EXPORT_GEOM_TGT ), default_geom_tangent ); }
bool Settings::get_export_geom_bnr (){ return mIOSettings->GetBoolProp( PROP_ID( EXPORT_GEOM_BNR ), default_geom_binorm  ); }
bool Settings::get_export_geom_clr (){ return mIOSettings->GetBoolProp( PROP_ID( EXPORT_GEOM_CLR ), default_geom_color   ); }
bool Settings::get_export_geom_skn (){ return mIOSettings->GetBoolProp( PROP_ID( EXPORT_GEOM_SKN ), default_geom_skin    ); }


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

void Settings::set_embed_textures( bool value){
    mIOSettings->SetBoolProp( EXP_EMBEDTEXTURE, value );
}

void Settings::set_export_geom_uv ( bool value){ mIOSettings->SetBoolProp( PROP_ID( EXPORT_GEOM_UV  ), value ); }
void Settings::set_export_geom_uv2( bool value){ mIOSettings->SetBoolProp( PROP_ID( EXPORT_GEOM_UV2 ), value ); }
void Settings::set_export_geom_nrm( bool value){ mIOSettings->SetBoolProp( PROP_ID( EXPORT_GEOM_NRM ), value ); }
void Settings::set_export_geom_tgt( bool value){ mIOSettings->SetBoolProp( PROP_ID( EXPORT_GEOM_TGT ), value ); }
void Settings::set_export_geom_bnr( bool value){ mIOSettings->SetBoolProp( PROP_ID( EXPORT_GEOM_BNR ), value ); }
void Settings::set_export_geom_clr( bool value){ mIOSettings->SetBoolProp( PROP_ID( EXPORT_GEOM_CLR ), value ); }
void Settings::set_export_geom_skn( bool value){ mIOSettings->SetBoolProp( PROP_ID( EXPORT_GEOM_SKN ), value ); }



AWD_field_type Settings::get_geoms_type(){
    if( get_wide_geoms() ){
        return AWD_FIELD_FLOAT64;
    } else {
        return AWD_FIELD_FLOAT32;
    }
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


FbxProperty Settings::geom_uv (FbxIOSettings* pIOS) { return pIOS->GetProperty( PROP_ID( EXPORT_GEOM_UV  ) ); }
FbxProperty Settings::geom_uv2(FbxIOSettings* pIOS) { return pIOS->GetProperty( PROP_ID( EXPORT_GEOM_UV2 ) ); }
FbxProperty Settings::geom_nrm(FbxIOSettings* pIOS) { return pIOS->GetProperty( PROP_ID( EXPORT_GEOM_NRM ) ); }
FbxProperty Settings::geom_tgt(FbxIOSettings* pIOS) { return pIOS->GetProperty( PROP_ID( EXPORT_GEOM_TGT ) ); }
FbxProperty Settings::geom_bnr(FbxIOSettings* pIOS) { return pIOS->GetProperty( PROP_ID( EXPORT_GEOM_BNR ) ); }
FbxProperty Settings::geom_clr(FbxIOSettings* pIOS) { return pIOS->GetProperty( PROP_ID( EXPORT_GEOM_CLR ) ); }
FbxProperty Settings::geom_skn(FbxIOSettings* pIOS) { return pIOS->GetProperty( PROP_ID( EXPORT_GEOM_SKN ) ); }



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

