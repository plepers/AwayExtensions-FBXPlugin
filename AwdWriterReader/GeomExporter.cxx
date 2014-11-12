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
     * Todo , return if geom already exported
     */
    
    FbxMesh* pMesh = (FbxMesh*) pObject;
//    FbxMesh *pMesh = (FbxMesh*) pNode;
    
    if (!pMesh->GetNode())
        return;
    
    const int lPolygonCount = pMesh->GetPolygonCount();
    
    // Empty Geom
    if (pMesh->GetControlPointsCount() == 0)
    {
        return;
    }
    
    
    
    
    // Count the polygon count of each material
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
    
    // All faces will use the same material.
    if (mSubMeshes.GetCount() == 0)
    {
        mSubMeshes.Resize(1);
        mSubMeshes[0] = new SubMesh();
    }
    
    
    
    
    
    
    // Congregate all the data of a mesh to be cached in VBOs.
    // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
    
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
    float * lVertices = new float[ lPolygonVertexCount * 3 ];
    unsigned int * lIndices = new unsigned int[lPolygonCount * TRIANGLE_VERTEX_COUNT];
    float * lNormals = NULL;
    if (mHasNormal)
    {
        lNormals = new float[lPolygonVertexCount * 3];
    }
    float * lUVs = NULL;
    FbxStringList lUVNames;
    pMesh->GetUVSetNames(lUVNames);
    const char * lUVName = NULL;
    if (mHasUV && lUVNames.GetCount())
    {
        lUVs = new float[lPolygonVertexCount * 2];
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
            lVertices[lIndex * 3] = static_cast<float>(lCurrentVertex[0]);
            lVertices[lIndex * 3 + 1] = static_cast<float>(lCurrentVertex[1]);
            lVertices[lIndex * 3 + 2] = static_cast<float>(lCurrentVertex[2]);
            
            // Save the normal.
            if (mHasNormal)
            {
                int lNormalIndex = lIndex;
                if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
                lNormals[lIndex * 3] = static_cast<float>(lCurrentNormal[0]);
                lNormals[lIndex * 3 + 1] = static_cast<float>(lCurrentNormal[1]);
                lNormals[lIndex * 3 + 2] = static_cast<float>(lCurrentNormal[2]);
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
                lUVs[lIndex * 2] = static_cast<float>(lCurrentUV[0]);
                lUVs[lIndex * 2 + 1] = static_cast<float>(lCurrentUV[1]);
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
                lVertices[lVertexCount * 3] = static_cast<float>(lCurrentVertex[0]);
                lVertices[lVertexCount * 3 + 1] = static_cast<float>(lCurrentVertex[1]);
                lVertices[lVertexCount * 3 + 2] = static_cast<float>(lCurrentVertex[2]);
                
                if (mHasNormal)
                {
                    pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
                    lNormals[lVertexCount * 3] = static_cast<float>(lCurrentNormal[0]);
                    lNormals[lVertexCount * 3 + 1] = static_cast<float>(lCurrentNormal[1]);
                    lNormals[lVertexCount * 3 + 2] = static_cast<float>(lCurrentNormal[2]);
                }
                
                if (mHasUV)
                {
                    bool lUnmappedUV;
                    pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
                    lUVs[lVertexCount * 2] = static_cast<float>(lCurrentUV[0]);
                    lUVs[lVertexCount * 2 + 1] = static_cast<float>(lCurrentUV[1]);
                }
            }
            ++lVertexCount;
        }
        mSubMeshes[lMaterialIndex]->TriangleCount += 1;
    }

    
    
    
    
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
    AWD_field_type precision_geo = AWD_FIELD_FLOAT32;
    
    
    
    int lsubIndex = 0;
    
    
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
    
    
    
    const char *name = pMesh->GetName();
    AWDTriGeom* geom = new AWDTriGeom( name, strlen( name ) );
    
    
    
    for ( lsubIndex = 0; lsubIndex<mSubMeshes.GetCount(); lsubIndex++) {
        if( mSubMeshes[lsubIndex]->TriangleCount > 0 ) {
            
            
            AWDBlockList *sgMatList = new AWDBlockList();
            AWDBlock *mat = matList->getByIndex(lsubIndex);
            if( mat )
                sgMatList->append( matList->getByIndex(lsubIndex) );
            // I don't why sub geom need a material list here,
            // but give him what he want
            AWDSubGeom* subGeom = new AWDSubGeom( sgMatList );
            
            // todo handle precision
            subGeom->add_stream(VERTICES, AWD_FIELD_FLOAT32, <#AWD_str_ptr#>, <#awd_uint32#>)
        }
    }
    
    
}