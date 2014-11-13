//
//  GeomExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 08/11/2014.
//
//

#include "GeomExporter.h"
#include "MaterialExporter.h"


const int TRIANGLE_VERTEX_COUNT = 3;

bool GeomExporter::isHandleObject( FbxObject *pObj ){
    return ( pObj->Is<FbxMesh>() );
}


void GeomExporter::doExport(FbxObject* pObject){
        
    /*
     * return if geom already exported
     */
    if( mBlocksMap->Get(pObject) ){
        return;
    }
    
    
    FbxMesh* pMesh = (FbxMesh*) pObject;
//    FbxMesh *pMesh = (FbxMesh*) pNode;
    
    if (!pMesh->GetNode())
        return;
    
    
    // triangulate mesh using FBX geometry converter
    // here we should be able to triangulate nurbs and patch too
    // but maybe we should use a dedicated NodeExporter
    //
    FbxGeometryConverter *geomConverter = new FbxGeometryConverter( mFbxManager );
    FbxNodeAttribute *nodeAttribute = geomConverter->Triangulate( pMesh, false );
    
    FBX_ASSERT_MSG( nodeAttribute != NULL, "Mesh triangulation failed");
    FBX_ASSERT_MSG( nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh, "Triangulated object isn't a eMesh");
    
    pMesh = (FbxMesh*) nodeAttribute;
    
    
    
    
    const int lPolygonCount = pMesh->GetPolygonCount();
    
    // Empty Geom
    if (pMesh->GetControlPointsCount() == 0)
    {
        return;
    }
    
    
    
    
    // Count the polygon count of each material
    // and create a list of subMeshes
    FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
    FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
    
    if (pMesh->GetElementMaterial())
    {
        lMaterialIndice = &pMesh->GetElementMaterial()->GetIndexArray();
        lMaterialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
        if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
        {
            FBX_ASSERT(lMaterialIndice->GetCount() == lPolygonCount);
            if (lMaterialIndice->GetCount() == lPolygonCount)
            {
                // Count the faces of each material
                for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
                {
                    const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
                    if (mSubMeshes.GetCount() < lMaterialIndex + 1)
                    {
                        mSubMeshes.Resize(lMaterialIndex + 1);
                    }
                    if (mSubMeshes[lMaterialIndex] == NULL)
                    {
                        mSubMeshes[lMaterialIndex] = new SubMesh;
                    }
                    mSubMeshes[lMaterialIndex]->TriangleCount += 1;
                }
                
                // Make sure we have no "holes" (NULL) in the mSubMeshes table. This can happen
                // if, in the loop above, we resized the mSubMeshes by more than one slot.
                for (int i = 0; i < mSubMeshes.GetCount(); i++)
                {
                    if (mSubMeshes[i] == NULL)
                        mSubMeshes[i] = new SubMesh;
                }
                
                // Record the offset (how many vertex)
                const int lMaterialCount = mSubMeshes.GetCount();
                int lOffset = 0;
                for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
                {
                    mSubMeshes[lIndex]->IndexOffset = lOffset;
                    lOffset += mSubMeshes[lIndex]->TriangleCount * 3;
                    // This will be used as counter in the following procedures, reset to zero
                    mSubMeshes[lIndex]->TriangleCount = 0;
                }
                FBX_ASSERT(lOffset == lPolygonCount * 3);
            }
        }
    }
    
    // no materials found on mesh
    // All faces will use the same material.
    if (mSubMeshes.GetCount() == 0)
    {
        mSubMeshes.Resize(1);
        mSubMeshes[0] = new SubMesh();
    }
    
    
    
    
    
    
    // Congregate all the data of a mesh to be cached in VBOs.
    // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
    // Here, all submeshes share the same VBOs, we will split them later
    
    mHasNormal = pMesh->GetElementNormalCount() > 0;
    mHasUV = pMesh->GetElementUVCount() > 0;
    
    FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
    
    if (mHasNormal)
    {
        lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
        if (lNormalMappingMode == FbxGeometryElement::eNone)
        {
            mHasNormal = false;
        }
        if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }
    if (mHasUV)
    {
        lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
        if (lUVMappingMode == FbxGeometryElement::eNone)
        {
            mHasUV = false;
        }
        if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }
    
    // Allocate the array memory, by control point or by polygon vertex.
    int lPolygonVertexCount = pMesh->GetControlPointsCount();
    if (!mAllByControlPoint)
    {
        lPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
    }
    awd_float64 * lVertices = new awd_float64[ lPolygonVertexCount * 3 ];
    unsigned int * lIndices = new unsigned int[lPolygonCount * TRIANGLE_VERTEX_COUNT];
    
    awd_float64 * lNormals = NULL;
    if (mHasNormal)
    {
        lNormals = new awd_float64[lPolygonVertexCount * 3];
    }
    
    awd_float64 * lUVs = NULL;
    FbxStringList lUVNames;
    pMesh->GetUVSetNames(lUVNames);
    const char * lUVName = NULL;
    if (mHasUV && lUVNames.GetCount())
    {
        lUVs = new awd_float64[lPolygonVertexCount * 2];
        lUVName = lUVNames[0];
    }


    
    // Populate the array with vertex attribute, if by control point.
    const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
    FbxVector4 lCurrentVertex;
    FbxVector4 lCurrentNormal;
    FbxVector2 lCurrentUV;
    if (mAllByControlPoint)
    {
        const FbxGeometryElementNormal * lNormalElement = NULL;
        const FbxGeometryElementUV * lUVElement = NULL;
        if (mHasNormal)
        {
            lNormalElement = pMesh->GetElementNormal(0);
        }
        if (mHasUV)
        {
            lUVElement = pMesh->GetElementUV(0);
        }
        for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
        {
            // Save the vertex position.
            lCurrentVertex = lControlPoints[lIndex];
            lVertices[lIndex * 3 + 0] = lCurrentVertex[0];
            lVertices[lIndex * 3 + 1] = lCurrentVertex[1];
            lVertices[lIndex * 3 + 2] = lCurrentVertex[2];
            
            // Save the normal.
            if (mHasNormal)
            {
                int lNormalIndex = lIndex;
                if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
                lNormals[lIndex * 3 + 0] = lCurrentNormal[0];
                lNormals[lIndex * 3 + 1] = lCurrentNormal[1];
                lNormals[lIndex * 3 + 2] = lCurrentNormal[2];
            }
            
            // Save the UV.
            if (mHasUV)
            {
                int lUVIndex = lIndex;
                if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
                lUVs[lIndex * 2 + 0] = lCurrentUV[0];
                lUVs[lIndex * 2 + 1] = lCurrentUV[1];
            }
        }
        
    }
    
    int lVertexCount = 0;
    
    
    for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
    {
        // The material for current face.
        int lMaterialIndex = 0;
        if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
        {
            lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
        }
        
        // Where should I save the vertex attribute index, according to the material
        const int lIndexOffset = mSubMeshes[lMaterialIndex]->IndexOffset +
        mSubMeshes[lMaterialIndex]->TriangleCount * 3;
        for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
        {
            const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
            
            if (mAllByControlPoint)
            {
                lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
            }
            // Populate the array with vertex attribute, if by polygon vertex.
            else
            {
                lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);
                
                lCurrentVertex = lControlPoints[lControlPointIndex];
                lVertices[lVertexCount * 3 + 0] = lCurrentVertex[0];
                lVertices[lVertexCount * 3 + 1] = lCurrentVertex[1];
                lVertices[lVertexCount * 3 + 2] = lCurrentVertex[2];
                
                if (mHasNormal)
                {
                    pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
                    lNormals[lVertexCount * 3 + 0] = lCurrentNormal[0];
                    lNormals[lVertexCount * 3 + 1] = lCurrentNormal[1];
                    lNormals[lVertexCount * 3 + 2] = lCurrentNormal[2];
                }
                
                if (mHasUV)
                {
                    bool lUnmappedUV;
                    pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
                    lUVs[lVertexCount * 2 + 0] = lCurrentUV[0];
                    lUVs[lVertexCount * 2 + 1] = lCurrentUV[1];
                }
            }
            ++lVertexCount;
        }
        mSubMeshes[lMaterialIndex]->TriangleCount += 1;
    }

    // todo , here we should collapse duplicated vertices
    // before generate finals submeshes
    
    //
    //
    //
    
    
    
    // Split vertices buffers by subMeshes
    
    // create temporary buffers with full geometry length
    // since we don't know yet number of vertices in each subGeoms
    
    unsigned int *tIndices;
    
    awd_float64 *tVertices = new awd_float64[ lPolygonVertexCount * 3 ];
    awd_float64 *tNormals;
    awd_float64 *tUVs;
    
    if (mHasNormal)
    {
        tNormals = new awd_float64[lPolygonVertexCount * 3];
    }
    if (mHasUV)
    {
        tUVs = new awd_float64[lPolygonVertexCount * 2];
    }
    
    int * tIdxMap = new int[lPolygonCount * TRIANGLE_VERTEX_COUNT];
    

    
    
    int lsubIndex = 0;
    int idxPtr;
    int newIdxPtr;
    int lRemappedIndex = 0;
    unsigned int lCurrentIndex = 0;
    unsigned int lsgNumVertices = 0;
    
    
    
    for ( lsubIndex = 0; lsubIndex<mSubMeshes.GetCount(); lsubIndex++)
    {
        if( mSubMeshes[lsubIndex]->TriangleCount > 0 )
        {
            
            // reset the indices map
            // and the number of vertices
            lsgNumVertices = 0;
            for( int i = 0; i < lPolygonCount*TRIANGLE_VERTEX_COUNT; i++)
            {
                tIdxMap[i] = -1;
            }
            
            const int numIndices 	= mSubMeshes[lsubIndex]->TriangleCount * TRIANGLE_VERTEX_COUNT;
            const int sgOffset 		= mSubMeshes[lsubIndex]->IndexOffset;
            
            tIndices = new unsigned int[ numIndices ];
            
            
            for( int newIdxPtr =0; newIdxPtr < numIndices; newIdxPtr++ )
            {
                idxPtr = newIdxPtr + sgOffset;
                
                lCurrentIndex = lIndices[ idxPtr ];
                lRemappedIndex = tIdxMap[ lCurrentIndex ];
                
                // first time we found this vertex
                // in this subGeom. add it to the temp VBO
                // and store the remapped index to IdxMap
                if( lRemappedIndex == -1 )
                {
                    lRemappedIndex = lsgNumVertices;
                    tIdxMap[ lCurrentIndex ] = lRemappedIndex;
                    
                    tVertices[lsgNumVertices * 3 + 0] = lVertices[lCurrentIndex * 3 + 0];
                    tVertices[lsgNumVertices * 3 + 1] = lVertices[lCurrentIndex * 3 + 1];
                    tVertices[lsgNumVertices * 3 + 2] = lVertices[lCurrentIndex * 3 + 2];
                    
                    if (mHasNormal)
                    {
                        tNormals[lsgNumVertices * 3 + 0] = lNormals[lCurrentIndex * 3 + 0];
                        tNormals[lsgNumVertices * 3 + 1] = lNormals[lCurrentIndex * 3 + 1];
                        tNormals[lsgNumVertices * 3 + 2] = lNormals[lCurrentIndex * 3 + 2];
                    }
                    
                    if (mHasUV)
                    {
                        tUVs[lsgNumVertices * 2 + 0] = lUVs[lCurrentIndex * 2 + 0];
                        tUVs[lsgNumVertices * 2 + 1] = lUVs[lCurrentIndex * 2 + 1];
                    }
                    
                    // grow the number of vertices for this geom
                    lsgNumVertices++;
                    
                }
                // the vertex is already added to VBO
                // just use the index of this vertex
                else
                {
                    
                }
                
                tIndices[ newIdxPtr ] = lRemappedIndex;
            }
            

            
            // allocate SubMesh data, since we know exact number
            // of vertices needed per sub geoms
            //
            SubMeshData *data = new SubMeshData();
            mSubMeshes[lsubIndex]->data = data;
            
            data->numVertices = lsgNumVertices;
            
            data->indices 	= tIndices;
            
            data->vertices 	= new awd_float64[lsgNumVertices * 3];
            data->normals 	= new awd_float64[lsgNumVertices * 3];
            data->uvs 		= new awd_float64[lsgNumVertices * 2];
            
            
            memcpy(data->vertices, 	tVertices, 	lsgNumVertices * 3 * sizeof( awd_float64 ) );
            memcpy(data->normals, 	tNormals, 	lsgNumVertices * 3 * sizeof( awd_float64 ) );
            memcpy(data->uvs	, 	tUVs, 		lsgNumVertices * 2 * sizeof( awd_float64 ) );
            
            
     		
            
        }
    }
    
    
    // free buffers
    //
    free( lVertices );
    free( lIndices );
    if( lNormals )
        free( lNormals );
    if( lUVs )
        free( lUVs );
    
    // free temp buffers
    //
    free( tVertices );
    if( tNormals )
	    free( tNormals );
    if( tUVs )
	    free( tUVs );
    
    
    
    
    
    
    
//
//    FbxGeometryConverter *converter = new FbxGeometryConverter( mFbxManager );
//
//    converter->EmulateNormalsByPolygonVertex( (FbxMesh*) pNode );
//
//    conveter->SplitMeshPerMaterial( )
//
    
    bool splitFacs  = false; // awdGeom->get_split_faces()
    bool forceSplit = false;
    bool useUV      = mHasUV;
    bool useSecUvs  = false;
    bool useNormals = mHasNormal;
    
    double nrmThreshold = 0.0;
    // TODO : handle hi precision
    AWD_field_type precision_geo = AWD_FIELD_FLOAT32;
    
    
    // create and setup a material exporter
    
    MaterialExporter *matExporter = new MaterialExporter();
    matExporter->setup(mAwd, mFbxManager, mBlocksMap );
    
    AWDBlockList *matList = new AWDBlockList();
    
    for ( lsubIndex = 0; lsubIndex<mSubMeshes.GetCount(); lsubIndex++) {
        if( mSubMeshes[lsubIndex]->TriangleCount > 0 ) {
            
            // material
            FbxSurfaceMaterial * lMaterial = pMesh->GetNode()->GetMaterial( lsubIndex );
            if( lMaterial ) {
                FBX_ASSERT( matExporter->isHandleObject( lMaterial ) );
            
                matExporter->doExport( lMaterial );
            
                AWDBlock *mat = mBlocksMap->Get( lMaterial );
                matList->append(mat);
            }
            else {
                // todo ?? use a default mat here?
                matList->append(NULL);
            }
            
            
        }
    }
    matExporter->release();
    
    
    
    const char *name = "gegeom";//pMesh->GetName();
    AWDTriGeom* geom = new AWDTriGeom( name, strlen( name ) );
    
    for ( lsubIndex = 0; lsubIndex<mSubMeshes.GetCount(); lsubIndex++) {
        FBXSDK_printf("    submesh num tris : %i\n",mSubMeshes[lsubIndex]->TriangleCount );
        if( mSubMeshes[lsubIndex]->TriangleCount > 0 ) {
            
            
            
            
            AWDBlockList *sgMatList = new AWDBlockList();
            AWDBlock *mat = matList->getByIndex(lsubIndex);
            if( mat )
                sgMatList->append( matList->getByIndex(lsubIndex) );
            // I don't why sub geom need a material list here,
            // but give him what he want
            AWDSubGeom* subGeom = new AWDSubGeom( sgMatList );
            
            AWD_str_ptr v_str;
            v_str.f64 = mSubMeshes[lsubIndex]->data->vertices;
            subGeom->add_stream(VERTICES, precision_geo, v_str, mSubMeshes[lsubIndex]->data->numVertices * 3);
            
            AWD_str_ptr i_str;
            i_str.ui32 = mSubMeshes[lsubIndex]->data->indices;
            subGeom->add_stream(TRIANGLES, AWD_FIELD_UINT16, i_str, mSubMeshes[lsubIndex]->TriangleCount * TRIANGLE_VERTEX_COUNT );
            
            if (mHasNormal)
            {
                AWD_str_ptr n_str;
                n_str.f64 = mSubMeshes[lsubIndex]->data->normals;
                subGeom->add_stream(VERTEX_NORMALS, precision_geo, n_str, mSubMeshes[lsubIndex]->data->numVertices * 3);
            }
            
            if (mHasUV)
            {
                AWD_str_ptr u_str;
                u_str.f64 = mSubMeshes[lsubIndex]->data->uvs;
                subGeom->add_stream(UVS, precision_geo, u_str, mSubMeshes[lsubIndex]->data->numVertices * 2);
            }
            
            geom->add_sub_mesh( subGeom );
            
        }
    }
    
    mBlocksMap->Set( pObject, geom );
    
    
    FBXSDK_printf("Geometry exported : %s\n", pObject->GetName() );
    
}