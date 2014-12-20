//
//  GeomExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 08/11/2014.
//
//

// TODO : test secondary uvs, add tangant and binormal
// TODO : add skinning support



#include "GeomExporter.h"
#include "MaterialExporter.h"
#include "utils.h"


const int TRIANGLE_VERTEX_COUNT = 3;



bool GeomExporter::isHandleObject( FbxObject *pObj ){
    return ( pObj->Is<FbxMesh>() );
}


void GeomExporter::doExport(FbxObject* pObject){
    
    //
    // this geom is already exported
    // skip now
    //
    if( mContext->GetBlocksMap()->Get(pObject) ){
        return;
    }
    
    
    FbxMesh* pMesh = (FbxMesh*) pObject;
    
    //
    // this geom is not used in scene graph
    //
    if (!pMesh->GetNode())
        return;
    
    
    // triangulate mesh using FBX geometry converter
    // here we should be able to triangulate nurbs and patch too
    // but maybe we should use a dedicated NodeExporter
    //
    
    FbxGeometryConverter *geomConverter = new FbxGeometryConverter( mContext->GetFbxManager() );
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
    // after a "collapsing" pass
    
    mHasNormal   = pMesh->GetElementNormalCount() > 0;
    mHasTangent  = pMesh->GetElementTangentCount() > 0;
    mHasUV       = pMesh->GetElementUVCount() > 0;
    mHasUV2      = pMesh->GetElementUVCount() > 1;
    mHasVC       = pMesh->GetElementVertexColorCount() > 0;
    
    FbxGeometryElement::EMappingMode lMappingMode = FbxGeometryElement::eNone;
    
    if (mHasNormal)
    {
        lMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            mHasNormal = false;
        }
        if (mHasNormal && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }
    if (mHasTangent)
    {
        lMappingMode = pMesh->GetElementTangent(0)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            mHasTangent = false;
        }
        if (mHasTangent && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }
    if (mHasUV)
    {
        lMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            mHasUV = false;
        }
        if (mHasUV && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }
    if (mHasUV2)
    {
        lMappingMode = pMesh->GetElementUV(1)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            mHasUV2 = false;
        }
        if (mHasUV2 && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            mAllByControlPoint = false;
        }
    }
    if (mHasVC)
    {
        lMappingMode = pMesh->GetElementVertexColor(0)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            mHasVC = false;
        }
        if (mHasVC && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            // Vertex Color seem to can't be stored per vertex.
            mHasVC = false;
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
    
    
    
    FBXSDK_printf("             HAS TGT (%i) %i \n", mHasTangent , pMesh->GetElementTangentCount());
    
    
    awd_float64 * lNormals = NULL;
    awd_float64 * lTangents = NULL;
    awd_float64 * lUVs = NULL;
    awd_float64 * lUV2s = NULL;
    awd_float64 * lVCs = NULL;
    
    if (mHasNormal)
    {
        lNormals = new awd_float64[lPolygonVertexCount * 3];
    }
    
    if (mHasTangent)
    {
        lTangents = new awd_float64[lPolygonVertexCount * 3];
    }
    
    if (mHasVC)
    {
        lVCs = new awd_float64[lPolygonVertexCount * 3];
    }
    
    
    FbxStringList lUVNames;
    pMesh->GetUVSetNames(lUVNames);
    
    const char * lUVName = NULL;
    if (mHasUV && lUVNames.GetCount())
    {
        lUVs = new awd_float64[lPolygonVertexCount * 2];
        lUVName = lUVNames[0];
    }

    const char * lUV2Name = NULL;
    if (mHasUV && lUVNames.GetCount() > 1)
    {
        lUV2s = new awd_float64[lPolygonVertexCount * 2];
        lUV2Name = lUVNames[1];
    }


    
    // Populate the array with vertex attribute, if by control point.
    const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
    FbxVector4 lCurrentVertex;
    FbxVector4 lCurrentNormal;
    FbxVector4 lCurrentTangent;
    FbxVector2 lCurrentUV;
    FbxColor   lCurrentVC;
    
    if (mAllByControlPoint)
    {
        const FbxGeometryElementNormal *        lNormalElement  = NULL;
        const FbxGeometryElementTangent *       lTangentElement = NULL;
        const FbxGeometryElementUV *            lUVElement      = NULL;
        const FbxGeometryElementUV *            lUV2Element     = NULL;
        const FbxGeometryElementVertexColor *   lVCElement     = NULL;
        
        if (mHasNormal)
        {
            lNormalElement = pMesh->GetElementNormal(0);
        }
        if (mHasTangent)
        {
            lTangentElement = pMesh->GetElementTangent(0);
        }
        if (mHasUV)
        {
            lUVElement = pMesh->GetElementUV(0);
        }
        if (mHasUV2)
        {
            lUV2Element = pMesh->GetElementUV(1);
        }
        if (mHasVC)
        {
            lVCElement = pMesh->GetElementVertexColor(0);
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
            
            // Save the tangent.
            if (mHasTangent)
            {
                int lTangentIndex = lIndex;
                if (lTangentElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lTangentIndex = lTangentElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentTangent = lTangentElement->GetDirectArray().GetAt(lTangentIndex);
                lTangents[lIndex * 3 + 0] = lCurrentTangent[0];
                lTangents[lIndex * 3 + 1] = lCurrentTangent[1];
                lTangents[lIndex * 3 + 2] = lCurrentTangent[2];
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
            if (mHasUV2)
            {
                int lUVIndex = lIndex;
                if (lUV2Element->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lUVIndex = lUV2Element->GetIndexArray().GetAt(lIndex);
                }
                lCurrentUV = lUV2Element->GetDirectArray().GetAt(lUVIndex);
                lUV2s[lIndex * 2 + 0] = lCurrentUV[0];
                lUV2s[lIndex * 2 + 1] = lCurrentUV[1];
            }
            if (mHasVC)
            {
                int lVCIndex = lIndex;
                if (lVCElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lVCIndex = lVCElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentVC = lVCElement->GetDirectArray().GetAt(lVCIndex);
                lVCs[lIndex * 3 + 0] = lCurrentVC.mRed;
                lVCs[lIndex * 3 + 1] = lCurrentVC.mGreen;
                lVCs[lIndex * 3 + 2] = lCurrentVC.mBlue;
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
            // note : vertex colors can't be retreived in this case ?
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
                
                if (mHasTangent)
                {
                    pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentTangent);
                    lTangents[lVertexCount * 3 + 0] = lCurrentTangent[0];
                    lTangents[lVertexCount * 3 + 1] = lCurrentTangent[1];
                    lTangents[lVertexCount * 3 + 2] = lCurrentTangent[2];
                }
                
                if (mHasUV)
                {
                    bool lUnmappedUV;
                    pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
                    lUVs[lVertexCount * 2 + 0] = lCurrentUV[0];
                    lUVs[lVertexCount * 2 + 1] = lCurrentUV[1];
                }
                if (mHasUV2)
                {
                    bool lUnmappedUV;
                    pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUV2Name, lCurrentUV, lUnmappedUV);
                    lUV2s[lVertexCount * 2 + 0] = lCurrentUV[0];
                    lUV2s[lVertexCount * 2 + 1] = lCurrentUV[1];
                }
            }
            ++lVertexCount;
        }
        mSubMeshes[lMaterialIndex]->TriangleCount += 1;
    }

    
    
    
    
    // collapse duplicated vertices
    // before generate finals submeshes
    //
    
    Collapser *collapser = new Collapser( lIndices, lPolygonCount * TRIANGLE_VERTEX_COUNT, lPolygonVertexCount );
    
    collapser->addStream( lVertices, 3 );
    
    if( mHasNormal )
        collapser->addStream( lNormals, 3 );
    
    if( mHasTangent )
        collapser->addStream( lTangents, 3 );
    
    if( mHasUV )
        collapser->addStream( lUVs, 2 );
    
    if( mHasUV2 )
        collapser->addStream( lUV2s, 2 );
    
    if( mHasVC )
        collapser->addStream( lVCs, 3 );
    
    
    collapser->collapse();
    
    delete collapser;
    collapser = NULL;
    
    
    
    
    
    
    
    // Split vertices buffers by subMeshes
    
    // create temporary buffers with full geometry length
    // since we don't know yet number of vertices in each subGeoms
    
    unsigned int *tIndices;
    
    awd_float64 *tVertices  = new awd_float64[ lPolygonVertexCount * 3 ];
    awd_float64 *tNormals   = NULL;
    awd_float64 *tTangents  = NULL;
    awd_float64 *tUVs       = NULL;
    awd_float64 *tUV2s      = NULL;
    awd_float64 *tVCs       = NULL;
    
    if (mHasNormal)
    {
        tNormals = new awd_float64[lPolygonVertexCount * 3];
    }
    if (mHasTangent)
    {
        tTangents = new awd_float64[lPolygonVertexCount * 3];
    }
    if (mHasUV)
    {
        tUVs = new awd_float64[lPolygonVertexCount * 2];
    }
    if (mHasUV2)
    {
        tUV2s = new awd_float64[lPolygonVertexCount * 2];
    }
    if (mHasVC)
    {
        tVCs = new awd_float64[lPolygonVertexCount * 3];
    }
    
    int * tIdxMap = new int[lPolygonCount * TRIANGLE_VERTEX_COUNT];
    

    
    
    int lsubIndex = 0;
    int idxPtr;
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
                    
                    if (mHasTangent)
                    {
                        tTangents[lsgNumVertices * 3 + 0] = tTangents[lCurrentIndex * 3 + 0];
                        tTangents[lsgNumVertices * 3 + 1] = tTangents[lCurrentIndex * 3 + 1];
                        tTangents[lsgNumVertices * 3 + 2] = tTangents[lCurrentIndex * 3 + 2];
                    }
                    
                    if (mHasVC)
                    {
                        tVCs[lsgNumVertices * 3 + 0] = tVCs[lCurrentIndex * 3 + 0];
                        tVCs[lsgNumVertices * 3 + 1] = tVCs[lCurrentIndex * 3 + 1];
                        tVCs[lsgNumVertices * 3 + 2] = tVCs[lCurrentIndex * 3 + 2];
                    }
                    
                    if (mHasUV)
                    {
                        tUVs[lsgNumVertices * 2 + 0] = lUVs[lCurrentIndex * 2 + 0];
                        tUVs[lsgNumVertices * 2 + 1] = lUVs[lCurrentIndex * 2 + 1];
                    }
                    if (mHasUV2)
                    {
                        tUV2s[lsgNumVertices * 2 + 0] = lUV2s[lCurrentIndex * 2 + 0];
                        tUV2s[lsgNumVertices * 2 + 1] = lUV2s[lCurrentIndex * 2 + 1];
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
            FBXSDK_printf("Geometry - create submesh (%i), num verts : %i \n", lsubIndex, lsgNumVertices );
            
            SubMeshData *data = new SubMeshData();
            mSubMeshes[lsubIndex]->data = data;
            
            data->numVertices = lsgNumVertices;
            
            data->indices 	= tIndices;
            
            
            data->vertices 	= new awd_float64[lsgNumVertices * 3];
            memcpy(data->vertices, 	tVertices, 	lsgNumVertices * 3 * sizeof( awd_float64 ) );
            
            if (mHasNormal) {
                data->normals 	= new awd_float64[lsgNumVertices * 3];
                memcpy(data->normals, 	tNormals, 	lsgNumVertices * 3 * sizeof( awd_float64 ) );
            }
            
            if (mHasTangent) {
                data->tangent 	= new awd_float64[lsgNumVertices * 3];
                memcpy(data->tangent, 	tTangents, 	lsgNumVertices * 3 * sizeof( awd_float64 ) );
            }
            
            if (mHasVC) {
                data->colors 	= new awd_float64[lsgNumVertices * 3];
                memcpy(data->colors, 	tVCs, 	lsgNumVertices * 3 * sizeof( awd_float64 ) );
            }
            
            if( mHasUV ) {
                data->uvs 		= new awd_float64[lsgNumVertices * 2];
                memcpy(data->uvs	, 	tUVs, 		lsgNumVertices * 2 * sizeof( awd_float64 ) );
            }
            
            if( mHasUV2 ) {
                data->uvs2 		= new awd_float64[lsgNumVertices * 2];
                memcpy(data->uvs2	, 	tUV2s, 		lsgNumVertices * 2 * sizeof( awd_float64 ) );
            }
            
            
            
            
     		
            
        }
    }
    
    
    // free buffers
    //
    free( lVertices );
    free( lIndices );
    if( lNormals )
        free( lNormals );
    if( lTangents )
        free( lTangents );
    if( lUVs )
        free( lUVs );
    if( lUV2s )
        free( lUV2s );
    if( lVCs )
        free( lVCs );
    
    // free temp buffers
    //
    free( tVertices );
    if( tNormals )
        free( tNormals );
    if( tTangents )
        free( tTangents );
    if( tUVs )
        free( tUVs );
    if( tUV2s )
        free( tUV2s );
    if( tVCs )
        free( tVCs );
    
    
    lVertices = NULL;
    lIndices = NULL;
    lNormals = NULL;
    lTangents = NULL;
    lUVs = NULL;
    lUV2s = NULL;
    lVCs = NULL;
    tVertices = NULL;
    tNormals = NULL;
    tTangents= NULL;
    tUVs = NULL;
    tUV2s = NULL;
    tVCs = NULL;
    
    
    
    
    
    

    
    // create and setup a material exporter
    //
    // Materials should be exported by MeshExporter
    // but AWDSubGeom need matList in constructor
    // so we export them here.
    
    MaterialExporter *matExporter = new MaterialExporter();
    matExporter->setup( mContext, mExporters );
    
    AWDBlockList *matList = new AWDBlockList();
    
    for ( lsubIndex = 0; lsubIndex<mSubMeshes.GetCount(); lsubIndex++) {
        if( mSubMeshes[lsubIndex]->TriangleCount > 0 ) {
            
            // material
            FbxSurfaceMaterial * lMaterial = pMesh->GetNode()->GetMaterial( lsubIndex );

            if( lMaterial ) {
                FBX_ASSERT( matExporter->isHandleObject( lMaterial ) );
            
                matExporter->doExport( lMaterial );
            
                AWDBlock *mat = mContext->GetBlocksMap()->Get( lMaterial );
                matList->append(mat);
            }
            else {
                // todo ?? use a default mat here?
                matList->append(NULL);
            }
            
            
        }
    }
    matExporter->release();
    
    
    
    
    
    
    AWD_field_type precision_geo = mContext->GetSettings()->get_geoms_type();
    
    AWDTriGeom* geom = new AWDTriGeom( NULL, 0 );
    AwdUtils::CopyNodeName( pMesh, geom );
    
    for ( lsubIndex = 0; lsubIndex<mSubMeshes.GetCount(); lsubIndex++) {
        FBXSDK_printf("    submesh num tris : %i\n",mSubMeshes[lsubIndex]->TriangleCount );
        if( mSubMeshes[lsubIndex]->TriangleCount > 0 ) {
            
            
            AWDBlockList *sgMatList = new AWDBlockList();
            AWDBlock *mat = matList->getByIndex(lsubIndex);
            if( mat )
                sgMatList->append( matList->getByIndex(lsubIndex) );
            
            // I don't know why subgeom need a material list here,
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
            if (mHasTangent)
            {
                AWD_str_ptr t_str;
                t_str.f64 = mSubMeshes[lsubIndex]->data->tangent;
                subGeom->add_stream(VERTEX_TANGENTS, precision_geo, t_str, mSubMeshes[lsubIndex]->data->numVertices * 3);
            }
            if (mHasVC)
            {
                AWD_str_ptr t_str;
                t_str.f64 = mSubMeshes[lsubIndex]->data->colors;
                subGeom->add_stream(COLORS, precision_geo, t_str, mSubMeshes[lsubIndex]->data->numVertices * 3);
            }
            
            if (mHasUV)
            {
                AWD_str_ptr u_str;
                u_str.f64 = mSubMeshes[lsubIndex]->data->uvs;
                subGeom->add_stream(UVS, precision_geo, u_str, mSubMeshes[lsubIndex]->data->numVertices * 2);
            }
            
            if (mHasUV2)
            {
                AWD_str_ptr su_str;
                su_str.f64 = mSubMeshes[lsubIndex]->data->uvs2;
                subGeom->add_stream(SUVS, precision_geo, su_str, mSubMeshes[lsubIndex]->data->numVertices * 2);
            }
            
            geom->add_sub_mesh( subGeom );
            
        }
    }
    
    mContext->add_mesh_data( geom, pMesh );
    
}


// ---------------------
// Collapser
//

Collapser::Collapser(unsigned int *pIndices, unsigned int pNumIndices, unsigned int pNumVertices )
{
    mIndices 		= pIndices;
    mNumIndices 	= pNumIndices;
    mNumVertices 	= pNumVertices;
    
    mRemapTable = new unsigned int[mNumVertices];

    
    // identity map
    //
    for (unsigned int i = 0; i < pNumVertices; i++) {
        mRemapTable[i] = i;
    }
    
}

Collapser::~Collapser()
{
    for(int i=0; i < mStreams.GetCount(); i++)
    {
        delete mStreams[i];
    }
    
    mStreams.Clear();
    
    delete [] mRemapTable;
}


void Collapser::addStream(awd_float64 *data, unsigned int csize)
{
    int numStreams = mStreams.GetCount();
    
    mStreams.Resize(numStreams+1 );
    
    mStreams[numStreams] = new Stream();
    
    mStreams[numStreams]->csize = csize;
    mStreams[numStreams]->data = data;
    
}

//
// brute force but simple collapsing
// should probably be optimized...
//
void Collapser::collapse()
{
    
    int numStreams = mStreams.GetCount();
    
    
    FBXSDK_printf("start collapse, num verts : %i \n", mNumVertices );
    
    unsigned int lCurrent = 0;
    unsigned int lCompare = 0;
    
    
    for ( lCurrent = 0; lCurrent < mNumVertices-1; lCurrent++ )
    {
        
        if( mRemapTable[lCurrent] != lCurrent )
        {
            // this vertex is already remapped to a previous one
            // skip
            // ----
            continue;
        }
        
        // compare current with all following vertices
        // -----
        for ( lCompare = lCurrent+1; lCompare < mNumVertices; )
        {
            
            
            // for current/compare we check equality of all streams / all components
            // loop in streams then components
            // -----
            for ( int streamIndex = 0; streamIndex < numStreams; streamIndex++ ) {
                
                
                
                unsigned int csize	= mStreams[streamIndex]->csize;
                awd_float64 *data 	= mStreams[streamIndex]->data;
                
                for (unsigned int comp = 0; comp < csize; comp++) {
                    
                    // Todo : compare with an epsilon ?
                    //
                    if( data[ lCurrent*csize + comp ] != data[ lCompare*csize + comp ] ){
                        // components are not equals
                        // skip this vertex, go to next one
                        // ------
                        goto nextVert;
                    }
                    
                    
                }
                
                
            }
            
    		// lCompare is the same vertex than lCurrent
            //
            mRemapTable[lCompare] = lCurrent;
            
            nextVert :
            lCompare++;
            
        }
        
    }
    
    
    unsigned int i;

    for (i = 0; i < mNumIndices; i++) {
        mIndices[i] = mRemapTable[ mIndices[i] ];
    }
    
    
//    int newLen = 0;
//    for (i = 0; i < mNumVertices; i++) {
//        if( mRemapTable[i] == i )
//            newLen++;
//    }
//    FBXSDK_printf("complete collapse, num verts : %i \n", newLen );
    
    
}




