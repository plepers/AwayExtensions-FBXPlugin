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
    
    FbxFileTexture * lTexture = FbxCast<FbxFileTexture>(pObj);
    
    const char * name = pObj->GetName();
    
    AWDBitmapTexture *awdTex = new AWDBitmapTexture( name, static_cast<unsigned short>( strlen( name ) ) );
    
    if( mContext->GetSettings()->get_embed_textures() )
    {
        awdTex->set_tex_type(EMBEDDED);
        // todo : embed texture if needed
    }
    else
    {
        awdTex->set_tex_type(EXTERNAL);
        
        // todo ? option for absolute / relative media paths
        FbxString url = lTexture->GetRelativeFileName();
        
        char *turl = (char*)malloc(url.Size()+1);
        strcpy( turl, url );
        
        //FBXSDK_printf("Export texture : external url : %s \n", url.Buffer() );
        awdTex->set_url( turl, static_cast<unsigned short>( url.Size() ) );
                        
    }
    
    // todo ? Several materials can hold different instances
    // of FbxFileTexture using the same file, maybe we should cache
    // textures using absolute file path as unique id, so we
    // don't export the same texture twice
    //
    mContext->add_texture( awdTex, pObj );
    
    
}
