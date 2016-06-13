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



#include <sys/time.h>

long getCurrentMs(){
    timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}


const int TRIANGLE_VERTEX_COUNT = 3;
const int MAX_BONES_PER_VERTEX = 4;


void GeomExporter::setup( ExportContext *context, ExporterProvider *provider )
{
    // call parent definition
    NodeExporter::setup( context, provider );
    
    // setup tootle lib
    tootleSettings.pMeshName             = NULL;
    tootleSettings.pViewpointName        = NULL;
    tootleSettings.nClustering           = 0;
    tootleSettings.nCacheSize            = TOOTLE_DEFAULT_VCACHE_SIZE;
    tootleSettings.eWinding              = TOOTLE_CW;
    tootleSettings.algorithmChoice       = TOOTLE_OPTIMIZE;
    tootleSettings.eVCacheOptimizer      = TOOTLE_VCACHE_AUTO;             // the auto selection as the default to optimize vertex cache
    tootleSettings.bOptimizeVertexMemory = true;                           // default value is to optimize the vertex memory
    tootleSettings.bMeasureOverdraw      = false;                           // default is to measure overdraw
    
    tootleSettings.bPrintStats           = false;

    TootleResult result;
    
    // initialize Tootle
    result = TootleInit();
    
    if (result != TOOTLE_OK)
    {
        DisplayTootleErrorMessage(result);
        exit(1);
    }
}



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

//    FBXSDK_printf("Export Geom %s\n", pMesh->GetNode()->GetName() );

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


    FbxArray<SubMesh*> mSubMeshes;


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


    bool lAllByControlPoint = true;

    bool lHasNormal   = mContext->GetSettings()->get_export_geom_nrm() && pMesh->GetElementNormalCount() > 0;
    bool lHasTangent  = mContext->GetSettings()->get_export_geom_tgt() && pMesh->GetElementTangentCount() > 0;
    bool lHasBinorm   = mContext->GetSettings()->get_export_geom_bnr() && pMesh->GetElementBinormalCount() > 0;
    bool lHasUV       = mContext->GetSettings()->get_export_geom_uv () && pMesh->GetElementUVCount() > 0;
    bool lHasUV2      = mContext->GetSettings()->get_export_geom_uv2() && pMesh->GetElementUVCount() > 1;
    bool lHasVC       = mContext->GetSettings()->get_export_geom_clr() && pMesh->GetElementVertexColorCount() > 0;
    bool lHasSkin     = mContext->GetSettings()->get_export_geom_skn() && pMesh->GetDeformerCount( FbxDeformer::eSkin ) > 0;

    FbxGeometryElement::EMappingMode lMappingMode = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode vcMappingMode = FbxGeometryElement::eNone;

    if (lHasNormal)
    {
        lMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            lHasNormal = false;
        }
        if (lHasNormal && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            lAllByControlPoint = false;
        }
    }
    if (lHasTangent)
    {
        lMappingMode = pMesh->GetElementTangent(0)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            lHasTangent = false;
        }
        if (lHasTangent && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            lAllByControlPoint = false;
        }
    }
    if (lHasBinorm)
    {
        lMappingMode = pMesh->GetElementBinormal(0)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            lHasBinorm = false;
        }
        if (lHasBinorm && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            lAllByControlPoint = false;
        }
    }
    if (lHasUV)
    {
        lMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            lHasUV = false;
        }
        if (lHasUV && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            lAllByControlPoint = false;
        }
    }
    if (lHasUV2)
    {
        lMappingMode = pMesh->GetElementUV(1)->GetMappingMode();
        if (lMappingMode == FbxGeometryElement::eNone)
        {
            lHasUV2 = false;
        }
        if (lHasUV2 && lMappingMode != FbxGeometryElement::eByControlPoint)
        {
            lAllByControlPoint = false;
        }
    }
    if (lHasVC)
    {
        lMappingMode = pMesh->GetElementVertexColor(0)->GetMappingMode();
        vcMappingMode = lMappingMode;

        if (lMappingMode == FbxGeometryElement::eNone)
        {
            lHasVC = false;
        }
        if (lHasVC && lMappingMode != FbxGeometryElement::eByControlPoint )
        {
            lAllByControlPoint = false;
        }
    }

    if( lHasSkin && !lAllByControlPoint ){
        // todo : if it can happen, find a way to export skin by polygon vertex
        FBXSDK_printf("WARN : skin can't be exported, not by control points" );
        lHasSkin = false;
    }



    // Allocate the array memory, by control point or by polygon vertex.
    int lPolygonVertexCount = pMesh->GetControlPointsCount();
    if (!lAllByControlPoint)
    {
        lPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
    }
    awd_float64 * lVertices = new awd_float64[ lPolygonVertexCount * 3 ];
    unsigned int * lIndices = new unsigned int[lPolygonCount * TRIANGLE_VERTEX_COUNT];



    awd_float64 * lNormals     = NULL;
    awd_float64 * lTangents    = NULL;
    awd_float64 * lBinorms     = NULL;
    awd_float64 * lUVs         = NULL;
    awd_float64 * lUV2s        = NULL;
    awd_float64 * lVCs         = NULL;
    awd_float64 * lSkinWeights = NULL;
    awd_uint32  * lSkinIndices = NULL;

    int lBonesPerVertex = MAX_BONES_PER_VERTEX;

    if (lHasNormal)
    {
        lNormals = new awd_float64[lPolygonVertexCount * 3];
    }

    if (lHasTangent)
    {
        lTangents = new awd_float64[lPolygonVertexCount * 3];
    }

    if (lHasBinorm)
    {
        lBinorms = new awd_float64[lPolygonVertexCount * 3];
    }

    if (lHasVC)
    {
        lVCs = new awd_float64[lPolygonVertexCount * 3];
    }

    if( lHasSkin ) {
        lSkinWeights = new awd_float64[lPolygonVertexCount * lBonesPerVertex];
        lSkinIndices = new awd_uint32[lPolygonVertexCount * lBonesPerVertex];
    }

    FbxStringList lUVNames;
    pMesh->GetUVSetNames(lUVNames);

    const char * lUVName = NULL;
    if (lHasUV && lUVNames.GetCount())
    {
        lUVs = new awd_float64[lPolygonVertexCount * 2];
        lUVName = lUVNames[0];
    }

    const char * lUV2Name = NULL;
    if (lHasUV && lUVNames.GetCount() > 1)
    {
        lUV2s = new awd_float64[lPolygonVertexCount * 2];
        lUV2Name = lUVNames[1];
    }



    // Populate the array with vertex attribute, if by control point.
    const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
    FbxVector4 lCurrentVertex;
    FbxVector4 lCurrentNormal;
    FbxVector4 lCurrentTangent;
    FbxVector4 lCurrentBinorm;
    FbxVector2 lCurrentUV;
    FbxColor   lCurrentVC;


    const FbxGeometryElementVertexColor *	lVCElement 		= NULL;
    if (lHasVC)
    {
        lVCElement = pMesh->GetElementVertexColor(0);
    }
    
    const FbxGeometryElementTangent *	lTangentElement 		= NULL;
    if (lHasTangent)
    {
        lTangentElement = pMesh->GetElementTangent(0);
    }
    
    const FbxGeometryElementBinormal *	lBinormElement 		= NULL;
    if (lHasBinorm)
    {
        lBinormElement = pMesh->GetElementBinormal(0);
    }
    
    
    
    
    if (lAllByControlPoint)
    {
        const FbxGeometryElementNormal *        lNormalElement  = NULL;
        const FbxGeometryElementUV *            lUVElement      = NULL;
        const FbxGeometryElementUV *            lUV2Element     = NULL;


        if (lHasNormal)
        {
            lNormalElement = pMesh->GetElementNormal(0);
        }
        if (lHasUV)
        {
            lUVElement = pMesh->GetElementUV(0);
        }
        if (lHasUV2)
        {
            lUV2Element = pMesh->GetElementUV(1);
        }


        // skin export
        //

        if( lHasSkin )
        {
            FbxSkin *skin = (FbxSkin *) pMesh->GetDeformer( 0, FbxDeformer::eSkin );

            int lClusterCount = skin->GetClusterCount();
            int lClusterIndex;

            // first find maximum number of bones per vertex
            // for further allocations
            //
            char *numClusterPerVertex = new char[ lPolygonVertexCount ];
            memset( numClusterPerVertex, 0, lPolygonVertexCount * sizeof(char) );
            int lMaxInfluences = 0;

            for( lClusterIndex = 0; lClusterIndex != lClusterCount; ++lClusterIndex )
            {

                FbxCluster* lCluster = skin->GetCluster( lClusterIndex );

                FbxNode* link = lCluster->GetLink();
                FbxNodeAttribute* linkAttrib = link->GetNodeAttribute();
                FbxNodeAttribute::EType nType = linkAttrib->GetAttributeType();
                FbxString linkName = lCluster->GetLink()->GetName();
                FBXSDK_printf( "link name %s \n", linkName.Buffer() );



                int lClusterIndicesCount = lCluster->GetControlPointIndicesCount();
                int* lClusterVIndices = lCluster->GetControlPointIndices();
                double* lClusterVWeights = lCluster->GetControlPointWeights();
                int lCurrentClusterSlot;

                for(int k = 0; k < lClusterIndicesCount; k++)
                {
                    int lcvIndex = lClusterVIndices[k];
                    int vindex;
                    double lcvWeight = lClusterVWeights[k];
                    lCurrentClusterSlot = numClusterPerVertex[lcvIndex];

                    if( lCurrentClusterSlot >= MAX_BONES_PER_VERTEX )
                    {
                        // no more room for more bones (4 weights per vertex)
                        // find the lower weight and replace it, if found.

                        int nIndex = 0;
                        double lowerWeight = 999.0;

                        for( int lrep = 0; lrep < MAX_BONES_PER_VERTEX; lrep++ )
                        {
                            vindex = lcvIndex*MAX_BONES_PER_VERTEX + lrep;
                            if( lSkinWeights[vindex] < lowerWeight ) {
                                lowerWeight = lSkinWeights[vindex];
                                nIndex = lrep;
                            }
                        }

                        if( lowerWeight < lcvWeight ){
                            vindex = lcvIndex*MAX_BONES_PER_VERTEX + nIndex;
                            lSkinWeights[vindex] = lcvWeight;
                            lSkinIndices[vindex] = lClusterIndex;
                        }

                    } else
                    {
                        vindex = lcvIndex*MAX_BONES_PER_VERTEX + lCurrentClusterSlot;
                        lSkinWeights[vindex] = lcvWeight;
                        lSkinIndices[vindex] = lClusterIndex;
                    }


                    if( lMaxInfluences < lCurrentClusterSlot ){
                        lMaxInfluences = lCurrentClusterSlot;
                    }

                    numClusterPerVertex[lcvIndex]++;
                }

            }

            // if max influence is lower than default num bones per vertex
            // reallocate stream
            //

            lMaxInfluences++;

            if( lMaxInfluences < MAX_BONES_PER_VERTEX ) {

                awd_float64 * rSkinWeights = new awd_float64[lPolygonVertexCount * lMaxInfluences];
                awd_uint32   * rSkinIndices = new awd_uint32[lPolygonVertexCount * lMaxInfluences];

                for(int k = 0; k < lPolygonVertexCount; k++)
                {
                    memcpy( &rSkinWeights[k*lMaxInfluences], &lSkinWeights[k*MAX_BONES_PER_VERTEX], lMaxInfluences*sizeof(awd_float64) );
                    memcpy( &rSkinIndices[k*lMaxInfluences], &lSkinIndices[k*MAX_BONES_PER_VERTEX], lMaxInfluences*sizeof(awd_uint32) );
                }

                lBonesPerVertex = lMaxInfluences;

                delete lSkinIndices;
                delete lSkinWeights;

                lSkinWeights = rSkinWeights;
                lSkinIndices = rSkinIndices;

            }


            delete[] numClusterPerVertex;


        }


        // export vertices attributes, by control points
        //
        //

        for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
        {
            // Save the vertex position.
            lCurrentVertex = lControlPoints[lIndex];
            lVertices[lIndex * 3 + 0] = lCurrentVertex[0];
            lVertices[lIndex * 3 + 1] = lCurrentVertex[1];
            lVertices[lIndex * 3 + 2] = lCurrentVertex[2];

            // Save the normal.
            if (lHasNormal)
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
            if (lHasTangent)
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

            // Save the binormals.
            if (lHasBinorm)
            {
                int lBinormIndex = lIndex;
                if (lBinormElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                {
                    lBinormIndex = lBinormElement->GetIndexArray().GetAt(lIndex);
                }
                lCurrentBinorm = lBinormElement->GetDirectArray().GetAt(lBinormIndex);
                lBinorms[lIndex * 3 + 0] = lCurrentBinorm[0];
                lBinorms[lIndex * 3 + 1] = lCurrentBinorm[1];
                lBinorms[lIndex * 3 + 2] = lCurrentBinorm[2];
            }

            // Save the UV.
            if (lHasUV)
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
            if (lHasUV2)
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
            if (lHasVC)
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



    // test

//    if( lHasVC){
//
//        int c = lVCElement->GetDirectArray().GetCount();
//        FBXSDK_printf("num  vc layers  %i \n",pMesh->GetElementVertexColorCount());
//        for (int i = 0; i < c; i++) {
//            FBXSDK_printf(" vc  %i -- %f \n", i, lVCElement->GetDirectArray().GetAt(i).mRed );
//        }
//        c = lVCElement->GetIndexArray().GetCount();
//        for (int i = 0; i < c; i++) {
//            FBXSDK_printf(" XX  %i -- %i \n", i, lVCElement->GetIndexArray().GetAt(i) );
//        }
//    }



    // Loop in polygons to create indices buffer
    // Export attributes, if not by control points
    //
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


            if (lAllByControlPoint)
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


                if (lHasNormal)
                {
                    pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
                    lNormals[lVertexCount * 3 + 0] = lCurrentNormal[0];
                    lNormals[lVertexCount * 3 + 1] = lCurrentNormal[1];
                    lNormals[lVertexCount * 3 + 2] = lCurrentNormal[2];
                }

                if (lHasTangent)
                {
//                    pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentTangent);
//                    lTangents[lVertexCount * 3 + 0] = lCurrentTangent[0];
//                    lTangents[lVertexCount * 3 + 1] = lCurrentTangent[1];
//                    lTangents[lVertexCount * 3 + 2] = lCurrentTangent[2];
                    int id = lVertexCount;
                    if( lTangentElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect ) {
                        id = lTangentElement->GetIndexArray().GetAt(id);
                    }
                    
                    lCurrentTangent = lTangentElement->GetDirectArray().GetAt(id);
                    
                    lTangents[lVertexCount * 3 + 0] = lCurrentTangent[0];
                    lTangents[lVertexCount * 3 + 1] = lCurrentTangent[1];
                    lTangents[lVertexCount * 3 + 2] = lCurrentTangent[2];
                }

                if (lHasBinorm)
                {
                    int id = lVertexCount;
                    if( lBinormElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect ) {
                        id = lBinormElement->GetIndexArray().GetAt(id);
                    }
                    
                    lCurrentBinorm = lBinormElement->GetDirectArray().GetAt(id);
                    
                    lBinorms[lVertexCount * 3 + 0] = lCurrentBinorm[0];
                    lBinorms[lVertexCount * 3 + 1] = lCurrentBinorm[1];
                    lBinorms[lVertexCount * 3 + 2] = lCurrentBinorm[2];
                }

                if (lHasUV)
                {
                    bool lUnmappedUV;
                    pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
                    lUVs[lVertexCount * 2 + 0] = lCurrentUV[0];
                    lUVs[lVertexCount * 2 + 1] = lCurrentUV[1];
                }
                if (lHasUV2)
                {
                    bool lUnmappedUV;
                    pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUV2Name, lCurrentUV, lUnmappedUV);
                    lUV2s[lVertexCount * 2 + 0] = lCurrentUV[0];
                    lUV2s[lVertexCount * 2 + 1] = lCurrentUV[1];
                }

                if (lHasVC)
                {
                    int id = lVertexCount;
                    if( lVCElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect ) {
                        id = lVCElement->GetIndexArray().GetAt(id);
                    }

                    lCurrentVC = lVCElement->GetDirectArray().GetAt(id);

                    lVCs[lVertexCount * 3 + 0] = lCurrentVC.mRed;
                    lVCs[lVertexCount * 3 + 1] = lCurrentVC.mGreen;
                    lVCs[lVertexCount * 3 + 2] = lCurrentVC.mBlue;
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

    if( lHasNormal )
        collapser->addStream( lNormals, 3 );

    if( lHasTangent )
        collapser->addStream( lTangents, 3 );

    if( lHasBinorm )
        collapser->addStream( lBinorms, 3 );

    if( lHasUV )
        collapser->addStream( lUVs, 2 );

    if( lHasUV2 )
        collapser->addStream( lUV2s, 2 );

    if( lHasVC )
        collapser->addStream( lVCs, 3 );

    if( lHasSkin )
        collapser->addStream( lSkinWeights, lBonesPerVertex );

    collapser->addStream( lVertices, 3 );

    collapser->collapse();

    delete collapser;
    collapser = NULL;







    // Split vertices buffers by subMeshes

    // create temporary buffers with full geometry length
    // since we don't know yet number of vertices in each subGeoms

    unsigned int *tIndices;

    awd_float64 *tVertices      = new awd_float64[ lPolygonVertexCount * 3 ];
    awd_float64 *tNormals       = NULL;
    awd_float64 *tTangents      = NULL;
    awd_float64 *tBinorms       = NULL;
    awd_float64 *tUVs           = NULL;
    awd_float64 *tUV2s          = NULL;
    awd_float64 *tVCs           = NULL;
    awd_float64 *tSkinWeights   = NULL;
    awd_uint32  *tSkinIndices   = NULL;

    if (lHasNormal)
    {
        tNormals = new awd_float64[lPolygonVertexCount * 3];
    }
    if (lHasTangent)
    {
        tTangents = new awd_float64[lPolygonVertexCount * 3];
    }
    if (lHasBinorm)
    {
        tBinorms = new awd_float64[lPolygonVertexCount * 3];
    }
    if (lHasUV)
    {
        tUVs = new awd_float64[lPolygonVertexCount * 2];
    }
    if (lHasUV2)
    {
        tUV2s = new awd_float64[lPolygonVertexCount * 2];
    }
    if (lHasVC)
    {
        tVCs = new awd_float64[lPolygonVertexCount * 3];
    }
    if (lHasSkin)
    {
        tSkinWeights = new awd_float64[lPolygonVertexCount * lBonesPerVertex];
        tSkinIndices = new awd_uint32[lPolygonVertexCount * lBonesPerVertex ];
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
            
//            FBXSDK_printf("   submesh %i\n", lsubIndex );
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

                    if (lHasNormal)
                    {
                        tNormals[lsgNumVertices * 3 + 0] = lNormals[lCurrentIndex * 3 + 0];
                        tNormals[lsgNumVertices * 3 + 1] = lNormals[lCurrentIndex * 3 + 1];
                        tNormals[lsgNumVertices * 3 + 2] = lNormals[lCurrentIndex * 3 + 2];
                    }

                    if (lHasTangent)
                    {
                        tTangents[lsgNumVertices * 3 + 0] = lTangents[lCurrentIndex * 3 + 0];
                        tTangents[lsgNumVertices * 3 + 1] = lTangents[lCurrentIndex * 3 + 1];
                        tTangents[lsgNumVertices * 3 + 2] = lTangents[lCurrentIndex * 3 + 2];
                    }

                    if (lHasBinorm)
                    {
                        tBinorms[lsgNumVertices * 3 + 0] = lBinorms[lCurrentIndex * 3 + 0];
                        tBinorms[lsgNumVertices * 3 + 1] = lBinorms[lCurrentIndex * 3 + 1];
                        tBinorms[lsgNumVertices * 3 + 2] = lBinorms[lCurrentIndex * 3 + 2];
                    }

                    if (lHasVC)
                    {
                        tVCs[lsgNumVertices * 3 + 0] = lVCs[lCurrentIndex * 3 + 0];
                        tVCs[lsgNumVertices * 3 + 1] = lVCs[lCurrentIndex * 3 + 1];
                        tVCs[lsgNumVertices * 3 + 2] = lVCs[lCurrentIndex * 3 + 2];
                    }

                    if (lHasUV)
                    {
                        tUVs[lsgNumVertices * 2 + 0] = lUVs[lCurrentIndex * 2 + 0];
                        tUVs[lsgNumVertices * 2 + 1] = lUVs[lCurrentIndex * 2 + 1];
                    }
                    if (lHasUV2)
                    {
                        tUV2s[lsgNumVertices * 2 + 0] = lUV2s[lCurrentIndex * 2 + 0];
                        tUV2s[lsgNumVertices * 2 + 1] = lUV2s[lCurrentIndex * 2 + 1];
                    }

                    if( lHasSkin )
                    {
                        memcpy( &tSkinWeights[lsgNumVertices*lBonesPerVertex], &lSkinWeights[lCurrentIndex*lBonesPerVertex], lBonesPerVertex*sizeof(awd_float64) );
                        memcpy( &tSkinIndices[lsgNumVertices*lBonesPerVertex], &lSkinIndices[lCurrentIndex*lBonesPerVertex], lBonesPerVertex*sizeof(awd_uint32) );
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
//            FBXSDK_printf("Geometry - create submesh (%i), num verts : %i \n", lsubIndex, lsgNumVertices );

            SubMeshData *data = new SubMeshData();
            mSubMeshes[lsubIndex]->data = data;
            
            data->numIndices = numIndices;
            data->numVertices = lsgNumVertices;
            data->bonesPerVertex = lBonesPerVertex;

            data->indices 	= tIndices;


            data->vertices 	= new awd_float64[lsgNumVertices * 3];
            memcpy(data->vertices, 	tVertices, 	lsgNumVertices * 3 * sizeof( awd_float64 ) );

            if (lHasNormal) {
                data->normals 	= new awd_float64[lsgNumVertices * 3];
                memcpy(data->normals, 	tNormals, 	lsgNumVertices * 3 * sizeof( awd_float64 ) );
            }

            if (lHasTangent) {
                data->tangent   = new awd_float64[lsgNumVertices * 3];
                memcpy(data->tangent,   tTangents,  lsgNumVertices * 3 * sizeof( awd_float64 ) );
            }

            if (lHasBinorm) {
                data->binorm   = new awd_float64[lsgNumVertices * 3];
                memcpy(data->binorm,   tBinorms,  lsgNumVertices * 3 * sizeof( awd_float64 ) );
            }

            if (lHasVC) {
                data->colors 	= new awd_float64[lsgNumVertices * 3];
                memcpy(data->colors, 	tVCs, 	lsgNumVertices * 3 * sizeof( awd_float64 ) );
            }

            if( lHasUV ) {
                data->uvs 		= new awd_float64[lsgNumVertices * 2];
                memcpy(data->uvs	, 	tUVs, 		lsgNumVertices * 2 * sizeof( awd_float64 ) );
            }

            if( lHasUV2 ) {
                data->uvs2 		= new awd_float64[lsgNumVertices * 2];
                memcpy(data->uvs2	, 	tUV2s, 		lsgNumVertices * 2 * sizeof( awd_float64 ) );
            }
            if( lHasSkin ) {
                data->skinWeights 	= new awd_float64[lsgNumVertices * lBonesPerVertex];
                data->skinIndices 	= new awd_uint32[lsgNumVertices * lBonesPerVertex];
                memcpy(data->skinWeights	, 	tSkinWeights, 		lsgNumVertices * lBonesPerVertex * sizeof( awd_float64 ) );
                memcpy(data->skinIndices, 	tSkinIndices, 		lsgNumVertices * lBonesPerVertex * sizeof( awd_uint32 ) );
            }
            
            
            // Tootle optimization
            // ------------------
            
            if( mContext->GetSettings()->get_tootle_optims() )
            {
                ProcessTootleSubMeshData( data, tootleSettings );
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
    if( lBinorms )
        free( lBinorms );
    if( lUVs )
        free( lUVs );
    if( lUV2s )
        free( lUV2s );
    if( lVCs )
        free( lVCs );
    if( lSkinWeights )
        free( lSkinWeights );
    if( lSkinIndices )
        free( lSkinIndices );

    // free temp buffers
    //
    free( tVertices );
    if( tNormals )
        free( tNormals );
    if( tTangents )
        free( tTangents );
    if( tBinorms )
        free( tBinorms );
    if( tUVs )
        free( tUVs );
    if( tUV2s )
        free( tUV2s );
    if( tVCs )
        free( tVCs );
    if( tSkinWeights )
        free( tSkinWeights );
    if( tSkinIndices )
        free( tSkinIndices );


    lVertices = NULL;
    lIndices = NULL;
    lNormals = NULL;
    lTangents = NULL;
    lBinorms = NULL;
    lUVs = NULL;
    lUV2s = NULL;
    lVCs = NULL;
    lSkinWeights = NULL;
    lSkinIndices = NULL;
    tVertices = NULL;
    tNormals = NULL;
    tTangents= NULL;
    tBinorms = NULL;
    tUVs = NULL;
    tUV2s = NULL;
    tVCs = NULL;
    tSkinWeights = NULL;
    tSkinIndices = NULL;








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


        } else {
            matList->append(NULL);
        }
    }
    matExporter->release();






    AWD_field_type precision_geo = mContext->GetSettings()->get_geoms_type();

    AWDTriGeom* geom = new AWDTriGeom( NULL, 0 );
    AwdUtils::CopyNodeName( pObject, geom );

    for ( lsubIndex = 0; lsubIndex<mSubMeshes.GetCount(); lsubIndex++) {
        if( mSubMeshes[lsubIndex]->TriangleCount > 0 ) {


            AWDBlockList *sgMatList = new AWDBlockList();

            FbxSurfaceMaterial * lMaterial = pMesh->GetNode()->GetMaterial( lsubIndex );
            AWDBlock *mat = mContext->GetBlocksMap()->Get( lMaterial );

            if( mat ) {
                sgMatList->append( mat );
            }

            // I don't know why subgeom need a material list here,
            // but give him what he want
            AWDSubGeom* subGeom = new AWDSubGeom( sgMatList );

            AWD_str_ptr v_str;
            v_str.f64 = mSubMeshes[lsubIndex]->data->vertices;
            subGeom->add_stream(VERTICES, precision_geo, v_str, mSubMeshes[lsubIndex]->data->numVertices * 3);

            AWD_str_ptr i_str;
            i_str.ui32 = mSubMeshes[lsubIndex]->data->indices;
            subGeom->add_stream(TRIANGLES, AWD_FIELD_UINT16, i_str, mSubMeshes[lsubIndex]->TriangleCount * TRIANGLE_VERTEX_COUNT );

            if (lHasNormal)
            {
                AWD_str_ptr n_str;
                n_str.f64 = mSubMeshes[lsubIndex]->data->normals;
                subGeom->add_stream(VERTEX_NORMALS, precision_geo, n_str, mSubMeshes[lsubIndex]->data->numVertices * 3);
            }
            if (lHasTangent)
            {
                AWD_str_ptr t_str;
                t_str.f64 = mSubMeshes[lsubIndex]->data->tangent;
                subGeom->add_stream(VERTEX_TANGENTS, precision_geo, t_str, mSubMeshes[lsubIndex]->data->numVertices * 3);
            }
            if (lHasBinorm)
            {
                AWD_str_ptr t_str;
                t_str.f64 = mSubMeshes[lsubIndex]->data->binorm;
                subGeom->add_stream(BINORMS, precision_geo, t_str, mSubMeshes[lsubIndex]->data->numVertices * 3);
            }
            if (lHasVC)
            {
                AWD_str_ptr t_str;
                t_str.f64 = mSubMeshes[lsubIndex]->data->colors;
                subGeom->add_stream(COLORS, precision_geo, t_str, mSubMeshes[lsubIndex]->data->numVertices * 3);
            }

            if (lHasUV)
            {
                AWD_str_ptr u_str;
                u_str.f64 = mSubMeshes[lsubIndex]->data->uvs;
                subGeom->add_stream(UVS, precision_geo, u_str, mSubMeshes[lsubIndex]->data->numVertices * 2);
            }

            if (lHasUV2)
            {
                AWD_str_ptr su_str;
                su_str.f64 = mSubMeshes[lsubIndex]->data->uvs2;
                subGeom->add_stream(SUVS, precision_geo, su_str, mSubMeshes[lsubIndex]->data->numVertices * 2);
            }

            if (lHasSkin)
            {
                AWD_str_ptr sw_str;
                sw_str.f64 = mSubMeshes[lsubIndex]->data->skinWeights;
                subGeom->add_stream(VERTEX_WEIGHTS, precision_geo, sw_str, mSubMeshes[lsubIndex]->data->numVertices * lBonesPerVertex);

                AWD_str_ptr si_str;
                si_str.ui32 = mSubMeshes[lsubIndex]->data->skinIndices;
                subGeom->add_stream(JOINT_INDICES, AWD_FIELD_UINT16, si_str, mSubMeshes[lsubIndex]->data->numVertices * lBonesPerVertex);
            }

            geom->add_sub_mesh( subGeom );

        }
    }

    mSubMeshes.Clear();

    mContext->add_mesh_data( geom, pMesh );

}


// ---------------------
// Collapser
//

Collapser::Collapser(unsigned int *pIndices, unsigned int pNumIndices, unsigned int pNumVertices )
{
    mIndices 		= pIndices;
    mNumIndices     = pNumIndices;
    mNumVertices 	= pNumVertices;
    mVertexSize     = 0;

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

    mVertexSize += csize;

}


void Collapser::collapse()
{


    long time = getCurrentMs();

    const int lNumBuckets = 256;


    int numStreams = mStreams.GetCount();


    unsigned int lCurrent = 0;
    unsigned int lCompare = 0;


    // first create an interleaved version of geometry
    // to compare vertices in one memcmp call
    //

    awd_float64 *interleaved = new awd_float64[mVertexSize * mNumVertices];
    awd_float64 *ptr = interleaved;

    for ( lCurrent = 0; lCurrent < mNumVertices; lCurrent++ )
    {
        for ( int streamIndex = 0; streamIndex < numStreams; streamIndex++ )
        {
            unsigned int csize	= mStreams[streamIndex]->csize;
            memcpy( ptr, &mStreams[streamIndex]->data[lCurrent*csize], csize * sizeof(awd_float64) );
            ptr += csize;
        }

    }

    // hash and split vertices into bucket
    // for faster compare/collapse
    //
    int ** buckets;
    hash *hashList = new hash[ mNumVertices ];


    unsigned int *bucketCounts = new unsigned int[ lNumBuckets ];
    memset( bucketCounts, 0, lNumBuckets * sizeof(unsigned int) );


    // hash all vertices, store hashes and count vertices per buckets
    //

    for ( lCurrent = 0; lCurrent < mNumVertices; lCurrent++ )
    {
        hash vHash = hashVertex( (char*) &interleaved[lCurrent * mVertexSize], mVertexSize * sizeof(awd_float64) );
        vHash = vHash % lNumBuckets;
        hashList[lCurrent] = vHash;
        bucketCounts[vHash] ++;
    }


    // allocate buckets
    //
    buckets = (int **) malloc( lNumBuckets * sizeof(void *) );

    for( hash i = 0; i < lNumBuckets; i++ )
    {
        buckets[i] = new int[ bucketCounts[i] ];
    }

    // for each buckets, create le list of corresponding vertex indices
    //
    memset( bucketCounts, 0, lNumBuckets * sizeof(unsigned int) );

    for ( lCurrent = 0; lCurrent < mNumVertices; lCurrent++ )
    {
        hash vHash = hashList[lCurrent];
        buckets[vHash][ bucketCounts[vHash] ] = lCurrent;
        bucketCounts[vHash] ++;
    }



    // collapse each buckets
    //

    int lindexA, lindexB;
    int* bucket;

    for( hash i = 0; i < lNumBuckets; i++ )
    {
        bucket = buckets[i];
        int numVerts = bucketCounts[i];



        if( numVerts < 2 )
        {
            continue;
        }

        for ( lindexA = 0; lindexA < numVerts-1; lindexA++ )
        {

            lCurrent = bucket[lindexA];

            if( mRemapTable[lCurrent] != lCurrent )
            {
                // this vertex is already remapped to a previous one
                // skip
                // ----
                continue;
            }

            // compare current with all following vertices
            // -----
            for ( lindexB = lindexA+1; lindexB < numVerts; lindexB++ )
            {
                lCompare = bucket[lindexB];

                if(
                   memcmp(
                          &interleaved[lCurrent*mVertexSize],
                          &interleaved[lCompare*mVertexSize],
                          mVertexSize * sizeof(awd_float64)
                          ) == 0
                   )
                {
                    // vertices are equals

                    mRemapTable[lCompare] = lCurrent;
                };


            }

        }


    }



    // free memory

    for( int i = 0; i < lNumBuckets; i++ )
    {
        delete buckets[i];
    }

    delete buckets;
    delete[] bucketCounts;
    delete[] hashList;


    long elapsed = ( getCurrentMs() - time );
    //FBXSDK_printf("complete collapse %i verts in %li ms", mNumVertices, elapsed );

    // remap indices

    remap();

    //logStats();

}

void Collapser::remap()
{
    unsigned int i;

    for (i = 0; i < mNumIndices; i++) {
        mIndices[i] = mRemapTable[ mIndices[i] ];
    }
}

/*
 * FNV hash
 */
Collapser::hash Collapser::hashVertex( char * vPtr, int len )
{
    unsigned char *p = (unsigned char*) vPtr;
    hash h = 2166136261;
    int i;

    for ( i = 0; i < len; i++ )
        h = ( h * 16777619 ) ^ p[i];

    return h;
}

void Collapser::logStats()
{
    int newLen = 0;
    for (int i = 0; i < mNumVertices; i++) {
        //FBXSDK_printf("     %i  -  %i \n", i , mRemapTable[i]  );
        if( mRemapTable[i] == i )
            newLen++;
    }

    FBXSDK_printf("complete collapse, num verts : %i vs %i \n", newLen, mNumVertices );
}



