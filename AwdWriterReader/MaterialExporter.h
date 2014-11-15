//
//  MaterialExporter.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 11/11/2014.
//
//

#ifndef __FbxAwdExporter__MaterialExporter__
#define __FbxAwdExporter__MaterialExporter__

#include "NodeExporter.h"


class MaterialExporter : public NodeExporter
{
public:
    virtual bool isHandleObject( FbxObject* );
    virtual void doExport( FbxObject* );
    
private:
    struct ColorChannel
    {
        ColorChannel() : mTexture(0), factor(1.0)
        {
            mColor[0] = 0.0f;
            mColor[1] = 0.0f;
            mColor[2] = 0.0f;
            mColor[3] = 1.0f;
        }
        const FbxTexture *mTexture;
        float factor;
        float mColor[4];
    };
    
    void getMaterialProperty(const FbxSurfaceMaterial * pMaterial,
                        const char * pPropertyName,
                        const char * pFactorPropertyName,
                        ColorChannel *pChannel );
    
    inline awd_color colorFromChannel( ColorChannel *channel );
};


#endif /* defined(__FbxAwdExporter__MaterialExporter__) */
