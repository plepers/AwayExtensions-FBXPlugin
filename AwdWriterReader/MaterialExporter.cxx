//
//  MaterialExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 11/11/2014.
//
//

#include "MaterialExporter.h"
#include "TextureExporter.h"
#include "utils.h"

unsigned char f2b( float f ){
    f = fmax(0.0, fmin(1.0, f));
    return floor(f == 1.0 ? 255 : f * 256.0);
}

awd_color MaterialExporter::colorFromChannel( ColorChannel *channel ){
    unsigned char r = f2b( channel->mColor[0] );
    unsigned char g = f2b( channel->mColor[1] );
    unsigned char b = f2b( channel->mColor[2] );
    return (r<<16) | (g << 8) | b;
}



void setXformFromTexture( FbxTexture *tex, awd_float64 *mtx )
{
    
    double angle = tex->GetRotationW() / 180.0 * 3.1415926535;
    double cosA = cos( angle );
    double sinA = sin( angle );
    
    // assume order a,b,c,d,tx,ty
    mtx[0] = cosA  * tex->GetScaleU();
    mtx[1] = sinA  * tex->GetScaleU();
    mtx[2] = -sinA * tex->GetScaleV();
    mtx[3] = cosA  * tex->GetScaleV();
    mtx[4] = tex->GetTranslationU();
    mtx[5] = tex->GetTranslationV();
    
}




// Get specific property value and connected texture if any.
// Value = Property value * Factor property value (if no factor property, multiply by 1).
//
void MaterialExporter::getMaterialProperty(const FbxSurfaceMaterial * pMaterial,
                               const char * pPropertyName,
                               const char * pFactorPropertyName,
                               ColorChannel *pChannel )
{
    FbxDouble3 lResult(0, 0, 0);
    
    const FbxProperty lProperty         = pMaterial->FindProperty(pPropertyName);
    const FbxProperty lFactorProperty   = pMaterial->FindProperty(pFactorPropertyName);
    
    if (lProperty.IsValid() && lFactorProperty.IsValid())
    {
        lResult = lProperty.Get<FbxDouble3>();
        double lFactor = lFactorProperty.Get<FbxDouble>();
        pChannel->factor = (float)lFactor;
    }
    pChannel->mColor[0] = lResult[0];
    pChannel->mColor[1] = lResult[1];
    pChannel->mColor[2] = lResult[2];
    
    if (lProperty.IsValid())
    {
        const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
        
        FBXSDK_printf("   Material texture count : %s -- %i\n", pPropertyName, lTextureCount );
        if (lTextureCount)
        {
            FbxFileTexture* lTexture = lProperty.GetSrcObject<FbxFileTexture>();
            pChannel->mTexture = lTexture;
        }
    }
}




bool MaterialExporter::isHandleObject( FbxObject *pObj )
{
    return ( pObj->Is<FbxSurfaceMaterial>() );
}

void MaterialExporter::doExport(FbxObject *pObj )
{
    FBX_ASSERT( isHandleObject( pObj ) );
    
    FbxSurfaceMaterial* lMat = (FbxSurfaceMaterial*) pObj;
    
    const char* name = lMat->GetName();
    
    
    FBXSDK_printf("Export Material  %s\n", name );
    
    AWDMaterial *awdMat = new AWDMaterial( name, static_cast<unsigned short>(strlen(name)) );
    
    FBXSDK_printf("  material shading model : %s\n", lMat->ShadingModel.Get().Buffer() );
    
    
    
    // extract channels data from FBX material
    //
    ColorChannel mEmissive;
    ColorChannel mAmbient;
    ColorChannel mDiffuse;
    ColorChannel mSpecular;
    
    getMaterialProperty(lMat, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, &mEmissive );
    getMaterialProperty(lMat, FbxSurfaceMaterial::sAmbient,  FbxSurfaceMaterial::sAmbientFactor,  &mAmbient );
    getMaterialProperty(lMat, FbxSurfaceMaterial::sDiffuse,  FbxSurfaceMaterial::sDiffuseFactor,  &mDiffuse );
    getMaterialProperty(lMat, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, &mSpecular );
    
    double mShinness = 0.0;
    
    FbxProperty lShininessProperty = lMat->FindProperty(FbxSurfaceMaterial::sShininess);
    if (lShininessProperty.IsValid())
    {
        mShinness = lShininessProperty.Get<FbxDouble>();
    }
    

    
    if( mDiffuse.mTexture )
    {
        awdMat->set_type( AWD_MATTYPE_TEXTURE );
    }
    else
    {
        awdMat->set_type( AWD_MATTYPE_COLOR );
    }
    
    awdMat->set_color( colorFromChannel(&mDiffuse) );
    
    awdMat->set_ambientColor( colorFromChannel(&mAmbient) );
    awdMat->set_ambientStrength(mAmbient.factor );
    
    awdMat->set_specularColor( colorFromChannel(&mSpecular) );
    awdMat->set_specularStrength(mSpecular.factor );
    awdMat->set_glossStrength( (awd_uint16) mShinness );

    awdMat->set_is_faceted( false );
    
    
    //
    // export textures
    //
    bool lDiffuseRepeat = true;
    bool lDiffusePremult = false;
    double *uvmtxData = (double *)malloc(6*sizeof(double));
    
    if( mDiffuse.mTexture || mAmbient.mTexture || mSpecular.mTexture )
    {
        FBXSDK_printf("  material has textures");
        TextureExporter *texExporter = new TextureExporter();
        texExporter->setup( mContext, mExporters );
        
        if( mDiffuse.mTexture )
        {
            texExporter->doExport( mDiffuse.mTexture );
            awdMat->set_texture( (AWDBitmapTexture *)mContext->GetBlocksMap()->Get(mDiffuse.mTexture) );

            lDiffuseRepeat = mDiffuse.mTexture->WrapModeU.Get() == FbxFileTexture::eRepeat && mDiffuse.mTexture->WrapModeV.Get() == FbxFileTexture::eRepeat;
            lDiffusePremult = mDiffuse.mTexture->PremultiplyAlpha.Get();
            
            
            setXformFromTexture( mDiffuse.mTexture, uvmtxData );
            
        }
        
        if( mAmbient.mTexture )
        {
            texExporter->doExport( mAmbient.mTexture );
            awdMat->set_ambientTexture( (AWDBitmapTexture *)mContext->GetBlocksMap()->Get(mAmbient.mTexture) );
        }
        
        if( mSpecular.mTexture )
        {
            texExporter->doExport( mSpecular.mTexture );
            awdMat->set_specTexture( (AWDBitmapTexture *)mContext->GetBlocksMap()->Get(mSpecular.mTexture) );
        }
        
        texExporter->release();
        
        
    }

    if( ! AwdUtils::isMatrix2dIdentity(uvmtxData) )
    {
        awdMat->set_uv_transform_mtx( uvmtxData );
    }
    
    awdMat->set_premultiplied( lDiffusePremult );
    awdMat->set_repeat( lDiffuseRepeat );
    
    mContext->add_material(awdMat, pObj );
    
}