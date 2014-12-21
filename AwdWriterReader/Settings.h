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
    void set_compression    ( AWD_compression );
    
    bool get_wide_matrix    ();
    bool get_wide_geoms     ();
    bool get_wide_props     ();
    bool get_wide_attribs   ();
    bool get_export_empty   ();
    bool get_split_by_root  ();
    bool get_embed_textures ();
    double get_scale        ();
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
    
};

#endif
