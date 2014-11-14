//
//  MaterialExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 11/11/2014.
//
//

#include "MaterialExporter.h"



// Get specific property value and connected texture if any.
// Value = Property value * Factor property value (if no factor property, multiply by 1).
FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial,
                               const char * pPropertyName,
                               const char * pFactorPropertyName,
                               FbxFileTexture & pFileTexture)
{
    FbxDouble3 lResult(0, 0, 0);
    const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
    const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
    if (lProperty.IsValid() && lFactorProperty.IsValid())
    {
        lResult = lProperty.Get<FbxDouble3>();
        double lFactor = lFactorProperty.Get<FbxDouble>();
        if (lFactor != 1)
        {
            lResult[0] *= lFactor;
            lResult[1] *= lFactor;
            lResult[2] *= lFactor;
        }
    }
    
    if (lProperty.IsValid())
    {
        const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
        if (lTextureCount)
        {
            const FbxFileTexture* lTexture = lProperty.GetSrcObject<FbxFileTexture>();
            if (lTexture && lTexture->GetUserDataPtr())
            {
                //pFileTexture = (lTexture);
            }
        }
    }
    
    return lResult;
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
    
    AWDMaterial *awdMat = new AWDMaterial( name, static_cast<unsigned short>(strlen(name)) );
    
    // TODO : implement materials export
    
    mBlocksMap->Set( pObj, awdMat );
    mAwd->add_material( awdMat );
    
}