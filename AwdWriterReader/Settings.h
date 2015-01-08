//
//  Settings.H
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 02/11/2014.
//
//

#ifndef FbxAwdExporter_Settings_H
#define FbxAwdExporter_Settings_H


#include <fbxsdk.h>
#include <awd/awd.h>

#include "ExporterConfig.h"

#define WIDE_MATRIX         "precision_mtx"
#define WIDE_GEOMS          "precision_geo"
#define WIDE_PROPS          "precision_props"
#define WIDE_ATTIBS         "precision_attr"

#define EXPORT_SCALE        "export_scale"
#define AWD_COMPRESSION     "compression"
#define EXPORT_EMPTY        "export_empty"
#define SPLIT_BY_ROOT       "split_by_root"

#define EXPORT_GEOM_UV      "export_geom_uv"
#define EXPORT_GEOM_UV2     "export_geom_uv2"
#define EXPORT_GEOM_NRM     "export_geom_normal"
#define EXPORT_GEOM_TGT     "export_geom_tangent"
#define EXPORT_GEOM_CLR     "export_geom_color"
#define EXPORT_GEOM_SKN     "export_geom_skin"


#define AWD_OPTION_GROUP EXP_ADV_OPT_GRP

#define PROP_ID( _ID ) AWD_OPTION_GROUP "|" FbxAwdExporter_PLUGIN_NAME "|" _ID

class Settings
{
public:
    
    Settings( FbxIOSettings* );
    
    
    void setupBlockSettings( BlockSettings * );
    
    
    FbxProperty getSettingsGroup();
    
    void set_wide_matrix    ( bool );
    void set_wide_geoms     ( bool );
    void set_wide_props     ( bool );
    void set_wide_attribs   ( bool );
    void set_export_empty   ( bool );
    void set_split_by_root  ( bool );
    void set_embed_textures ( bool );
    void set_scale          ( double );
    
    void set_export_geom_uv ( bool );
    void set_export_geom_uv2( bool );
    void set_export_geom_nrm( bool );
    void set_export_geom_tgt( bool );
    void set_export_geom_clr( bool );
    void set_export_geom_skn( bool );
    
    void set_compression    ( AWD_compression );
    
    
    
    bool get_wide_matrix    ();
    bool get_wide_geoms     ();
    bool get_wide_props     ();
    bool get_wide_attribs   ();
    bool get_export_empty   ();
    bool get_split_by_root  ();
    bool get_embed_textures ();
    double get_scale        ();
    
    bool get_export_geom_uv ();
    bool get_export_geom_uv2 ();
    bool get_export_geom_nrm ();
    bool get_export_geom_tgt ();
    bool get_export_geom_clr ();
    bool get_export_geom_skn ();
    
    AWD_compression get_compression();
    
    
    
    AWD_field_type get_geoms_type();
    
    static void FillDefaultValues( FbxIOSettings * );
    
private:
    FbxIOSettings   *mIOSettings;
    
    
    static FbxProperty wide_matrix_property  (FbxIOSettings *pIOS);
    static FbxProperty wide_geoms_property   (FbxIOSettings *pIOS);
    static FbxProperty wide_props_property   (FbxIOSettings *pIOS);
    static FbxProperty wide_attribs_property (FbxIOSettings *pIOS);
    static FbxProperty export_empty_property (FbxIOSettings *pIOS);
    static FbxProperty split_by_root_property(FbxIOSettings *pIOS);
    static FbxProperty scale_property        (FbxIOSettings *pIOS);
    static FbxProperty compression_property  (FbxIOSettings *pIOS);
    static FbxProperty geom_uv               (FbxIOSettings *pIOS);
    static FbxProperty geom_uv2              (FbxIOSettings *pIOS);
    static FbxProperty geom_nrm              (FbxIOSettings *pIOS);
    static FbxProperty geom_tgt              (FbxIOSettings *pIOS);
    static FbxProperty geom_clr              (FbxIOSettings *pIOS);
    static FbxProperty geom_skn              (FbxIOSettings *pIOS);
    
};

#endif
