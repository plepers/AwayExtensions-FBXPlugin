//
//  TextureExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 15/11/2014.
//
//

#include "TextureExporter.h"


bool TextureExporter::isHandleObject( FbxObject *pObj )
{
    return ( pObj->Is<FbxFileTexture>() );
}

void TextureExporter::doExport(FbxObject* pObj )
{
    
    FbxFileTexture * lTexture = (FbxFileTexture*) pObj;
    
    const char * name = pObj->GetName();
    
    AWDBitmapTexture *awdTex = new AWDBitmapTexture( name, static_cast<unsigned short>( strlen( name ) ) );
    
    
}
