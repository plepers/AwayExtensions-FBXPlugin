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

#define WIDE_MATRIX         "precision_mtx"
#define WIDE_GEOMS          "precision_geo"
#define WIDE_PROPS          "precision_props"
#define WIDE_ATTIBS         "precision_attr"
#define EXPORT_SCALE        "export_scale"
#define AWD_COMPRESSION     "compression"

namespace AwdSettings {
    
    void setupAwd( FbxIOSettings *, AWD * );
    void setupBlockSettings( FbxIOSettings *, BlockSettings * );
    
    void FillFbxIOSettings(FbxIOSettings& );
    
    FbxProperty getSettingsGroup(FbxIOSettings* );
    
    bool get_wide_matrix    (FbxIOSettings* );
    bool get_wide_geoms     (FbxIOSettings* );
    bool get_wide_props     (FbxIOSettings* );
    bool get_wide_attribs   (FbxIOSettings* );
    double get_scale        (FbxIOSettings* );
    
    AWD_compression get_compression        (FbxIOSettings* );
    
    void set_wide_matrix    (FbxIOSettings*, bool );
    void set_wide_geoms     (FbxIOSettings*, bool );
    void set_wide_props     (FbxIOSettings*, bool );
    void set_wide_attribs   (FbxIOSettings*, bool );
    void set_scale          (FbxIOSettings*, double );
    
    void set_compression        (FbxIOSettings*, AWD_compression );
    
}

#endif
