//
//  tootle.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 11/06/2016.
//
//

#include <tootle.h>
#include <vector>
#include "Timer.h"

void DisplayTootleErrorMessage(TootleResult eResult)
{
    std::cerr << "Tootle returned error: " << std::endl;

    switch (eResult)
    {
        case NA_TOOTLE_RESULT:
            std::cerr << " NA_TOOTLE_RESULT"       << std::endl;
            break;

        case TOOTLE_OK:
            break;

        case TOOTLE_INVALID_ARGS:
            std::cerr << " TOOTLE_INVALID_ARGS"    << std::endl;
            break;

        case TOOTLE_OUT_OF_MEMORY:
            std::cerr << " TOOTLE_OUT_OF_MEMORY"   << std::endl;
            break;

        case TOOTLE_3D_API_ERROR:
            std::cerr << " TOOTLE_3D_API_ERROR"    << std::endl;
            break;

        case TOOTLE_INTERNAL_ERROR:
            std::cerr << " TOOTLE_INTERNAL_ERROR"  << std::endl;
            break;

        case TOOTLE_NOT_INITIALIZED:
            std::cerr << " TOOTLE_NOT_INITIALIZED" << std::endl;
            break;
    }
}



int ProcessTootleSubMeshData( SubMeshData *subGeom, TootleSettings& settings )
{


    // *****************************************************************
    //   convert awd_float64 position buffer to float
    // *****************************************************************

    float*        pfVB      = (float*) malloc( subGeom->numVertices * 3 * sizeof(float) );
    for (int i=0; i < subGeom->numVertices * 3; i++ ) {
        pfVB[i] = (float)subGeom->vertices[i];
    }


    // *****************************************************************
    //   Prepare the mesh and initialize stats variables
    // *****************************************************************

    unsigned int  nFaces    = (unsigned int)  subGeom->numIndices/ 3;
    unsigned int  nVertices = subGeom->numVertices;
    unsigned int* pnIB      = subGeom->indices;
    unsigned int  nStride   = 3 * sizeof(float);

    TootleStats stats;

    // initialize the timing variables
    stats.fOptimizeVCacheTime               = INVALID_TIME;
    stats.fClusterMeshTime                  = INVALID_TIME;
    stats.fVCacheClustersTime               = INVALID_TIME;
    stats.fOptimizeVCacheAndClusterMeshTime = INVALID_TIME;
    stats.fOptimizeOverdrawTime             = INVALID_TIME;
    stats.fTootleOptimizeTime               = INVALID_TIME;
    stats.fTootleFastOptimizeTime           = INVALID_TIME;
    stats.fMeasureOverdrawTime              = INVALID_TIME;
    stats.fOptimizeVertexMemoryTime         = INVALID_TIME;


    // Viewpoints - NULL for now
    // ------------------
    const float* pViewpoints = NULL;
    unsigned int nViewpoints = 0;


    TootleResult result;

    // measure input VCache efficiency
    result = TootleMeasureCacheEfficiency(pnIB, nFaces, settings.nCacheSize, &stats.fVCacheIn);

    if (result != TOOTLE_OK)
    {
        DisplayTootleErrorMessage(result);
        return 1;
    }

    if (settings.bMeasureOverdraw)
    {
        // measure input overdraw.  Note that we assume counter-clockwise vertex winding.
        result = TootleMeasureOverdraw(pfVB, pnIB, nVertices, nFaces, nStride, pViewpoints, nViewpoints, settings.eWinding,
                                       &stats.fOverdrawIn, &stats.fMaxOverdrawIn);

        if (result != TOOTLE_OK)
        {
            DisplayTootleErrorMessage(result);
            return 1;
        }
    }


    // allocate an array to hold the cluster ID for each face
    std::vector<unsigned int> faceClusters;
    faceClusters.resize(nFaces + 1);
    unsigned int nNumClusters = 0;

    Timer timer;
    timer.Reset();

    // **********************************************************************************************************************
    //   Optimize the mesh:
    //
    // The following cases show five examples for developers on how to use the library functions in Tootle.
    // 1. If you are interested in optimizing vertex cache only, see the TOOTLE_VCACHE_ONLY case.
    // 2. If you are interested to optimize vertex cache and overdraw, see either TOOTLE_CLUSTER_VCACHE_OVERDRAW
    //     or TOOTLE_OPTIMIZE cases.  The former uses three separate function calls while the latter uses a single
    //     utility function.
    // 3. To use the algorithm from SIGGRAPH 2007 (v2.0), see TOOTLE_FAST_VCACHECLUSTER_OVERDRAW or TOOTLE_FAST_OPTIMIZE
    //     cases.  The former uses two separate function calls while the latter uses a single utility function.
    //
    // Note the algorithm from SIGGRAPH 2007 (v2.0) is very fast but produces less quality results especially for the
    //  overdraw optimization.  During our experiments with some medium size models, we saw an improvement of 1000x in
    //  running time (from 20+ minutes to less than 1 second) for using v2.0 calls vs v1.2 calls.  The resulting vertex
    //  cache optimization is very similar while the overdraw optimization drops from 3.8x better to 2.5x improvement over
    //  the input mesh.
    //  Developers should always run the overdraw optimization using the fast algorithm from SIGGRAPH initially.
    //  If they require a better result, then re-run the overdraw optimization using the old v1.2 path (TOOTLE_OVERDRAW_AUTO).
    //  Passing TOOTLE_OVERDRAW_AUTO to the algorithm will let the algorithm choose between Direct3D or raytracing path
    //  depending on the total number of clusters (less than 200 clusters, it will use Direct3D path otherwise it will
    //  use raytracing path since the raytracing path will be faster than the Direct3D path at that point).
    //
    // Tips: use v2.0 for fast optimization, and v1.2 to further improve the result by mix-matching the calls.
    // **********************************************************************************************************************

    switch (settings.algorithmChoice)
    {
        case TOOTLE_VCACHE_ONLY:
            // *******************************************************************************************************************
            // Perform Vertex Cache Optimization ONLY
            // *******************************************************************************************************************

            stats.nClusters = 1;

            // Optimize vertex cache
            result = TootleOptimizeVCache(pnIB, nFaces, nVertices, settings.nCacheSize,
                                          pnIB, NULL, settings.eVCacheOptimizer);

            if (result != TOOTLE_OK)
            {
                DisplayTootleErrorMessage(result);
                return 1;
            }

            stats.fOptimizeVCacheTime = timer.GetElapsed();
            break;

        case TOOTLE_CLUSTER_VCACHE_OVERDRAW:
            // *******************************************************************************************************************
            // An example of calling clustermesh, vcacheclusters and optimize overdraw individually.
            // This case demonstrate mix-matching v1.2 clustering with v2.0 overdraw optimization.
            // *******************************************************************************************************************

            // Cluster the mesh, and sort faces by cluster.
            result = TootleClusterMesh(pfVB, pnIB, nVertices, nFaces, nStride, settings.nClustering, pnIB, &faceClusters[0], NULL);

            if (result != TOOTLE_OK)
            {
                DisplayTootleErrorMessage(result);
                return 1;
            }

            stats.fClusterMeshTime = timer.GetElapsed();
            timer.Reset();

            // The last entry of of faceClusters store the total number of clusters.
            stats.nClusters = faceClusters[ nFaces ];

            // Perform vertex cache optimization on the clustered mesh.
            result = TootleVCacheClusters(pnIB, nFaces, nVertices, settings.nCacheSize, &faceClusters[0],
                                          pnIB, NULL, settings.eVCacheOptimizer);

            if (result != TOOTLE_OK)
            {
                DisplayTootleErrorMessage(result);
                return 1;
            }

            stats.fVCacheClustersTime = timer.GetElapsed();
            timer.Reset();

            // Optimize the draw order (using v1.2 path: TOOTLE_OVERDRAW_AUTO, the default path is from v2.0--SIGGRAPH version).
            result = TootleOptimizeOverdraw(pfVB, pnIB, nVertices, nFaces, nStride, pViewpoints, nViewpoints, settings.eWinding,
                                            &faceClusters[0], pnIB, NULL, TOOTLE_OVERDRAW_AUTO);

            if (result != TOOTLE_OK)
            {
                DisplayTootleErrorMessage(result);
                return 1;
            }

            stats.fOptimizeOverdrawTime = timer.GetElapsed();
            break;

        case TOOTLE_FAST_VCACHECLUSTER_OVERDRAW:
            // *******************************************************************************************************************
            // An example of calling v2.0 optimize vertex cache and clustering mesh with v1.2 overdraw optimization.
            // *******************************************************************************************************************

            // Optimize vertex cache and create cluster
            // The algorithm from SIGGRAPH combine the vertex cache optimization and clustering mesh into a single step
            result = TootleFastOptimizeVCacheAndClusterMesh(pnIB, nFaces, nVertices, settings.nCacheSize, pnIB,
                                                            &faceClusters[0], &nNumClusters, TOOTLE_DEFAULT_ALPHA);

            if (result != TOOTLE_OK)
            {
                // an error detected
                DisplayTootleErrorMessage(result);
                return 1;
            }

            stats.fOptimizeVCacheAndClusterMeshTime = timer.GetElapsed();
            timer.Reset();

            stats.nClusters = nNumClusters;

            // In this example, we use TOOTLE_OVERDRAW_AUTO to show that we can mix-match the clustering and
            //  vcache computation from the new library with the overdraw optimization from the old library.
            //  TOOTLE_OVERDRAW_AUTO will choose between using Direct3D or CPU raytracing path.  This path is
            //  much slower than TOOTLE_OVERDRAW_FAST but usually produce 2x better results.
            result = TootleOptimizeOverdraw(pfVB, pnIB, nVertices, nFaces, nStride, NULL, 0,
                                            settings.eWinding, &faceClusters[0], pnIB, NULL, TOOTLE_OVERDRAW_AUTO);

            if (result != TOOTLE_OK)
            {
                // an error detected
                DisplayTootleErrorMessage(result);
                return 1;
            }

            stats.fOptimizeOverdrawTime = timer.GetElapsed();

            break;

        case TOOTLE_OPTIMIZE:
            // *******************************************************************************************************************
            // An example of using a single utility function to perform v1.2 optimizations.
            // *******************************************************************************************************************

            // This function will compute the entire optimization (cluster mesh, vcache per cluster, and optimize overdraw).
            // It will use TOOTLE_OVERDRAW_FAST as the default overdraw optimization
            result = TootleOptimize(pfVB, pnIB, nVertices, nFaces, nStride, settings.nCacheSize,
                                    pViewpoints, nViewpoints, settings.eWinding, pnIB, &nNumClusters, settings.eVCacheOptimizer);

            if (result != TOOTLE_OK)
            {
                DisplayTootleErrorMessage(result);
                return 1;
            }

            stats.fTootleOptimizeTime = timer.GetElapsed();

            stats.nClusters = nNumClusters;
            break;

        case TOOTLE_FAST_OPTIMIZE:
            // *******************************************************************************************************************
            // An example of using a single utility function to perform v2.0 optimizations.
            // *******************************************************************************************************************

            // This function will compute the entire optimization (optimize vertex cache, cluster mesh, and optimize overdraw).
            // It will use TOOTLE_OVERDRAW_FAST as the default overdraw optimization
            result = TootleFastOptimize(pfVB, pnIB, nVertices, nFaces, nStride, settings.nCacheSize,
                                        settings.eWinding, pnIB, &nNumClusters, TOOTLE_DEFAULT_ALPHA);

            if (result != TOOTLE_OK)
            {
                DisplayTootleErrorMessage(result);
                return 1;
            }

            stats.fTootleFastOptimizeTime = timer.GetElapsed();

            stats.nClusters = nNumClusters;

            break;

        default:
            // wrong algorithm choice
            break;
    }


    // measure output VCache efficiency
    result = TootleMeasureCacheEfficiency(pnIB, nFaces, settings.nCacheSize, &stats.fVCacheOut);

    if (result != TOOTLE_OK)
    {
        DisplayTootleErrorMessage(result);
        return 1;
    }

    if (settings.bMeasureOverdraw)
    {
        // measure output overdraw
        timer.Reset();
        result = TootleMeasureOverdraw(pfVB, pnIB, nVertices, nFaces, nStride, pViewpoints, nViewpoints, settings.eWinding,
                                       &stats.fOverdrawOut, &stats.fMaxOverdrawOut);
        stats.fMeasureOverdrawTime = timer.GetElapsed();

        if (result != TOOTLE_OK)
        {
            DisplayTootleErrorMessage(result);
            return 1;
        }
    }


    // release float position buffer
    free( pfVB );


    //-----------------------------------------------------------------------------------------------------
    // PERFORM VERTEX MEMORY OPTIMIZATION (rearrange memory layout for vertices based on the final indices
    //  to exploit vertex cache prefetch).
    //  We want to optimize the vertex memory locations based on the final optimized index buffer that will
    //  be in the output file.
    //-----------------------------------------------------------------------------------------------------


    if (settings.bOptimizeVertexMemory)
    {

        timer.Reset();
        // ----------------
        // run Tootle optim
        //-----------------

        OptimizeVertexMemory( subGeom );
        stats.fOptimizeVertexMemoryTime = timer.GetElapsed();


    }
    
    if( settings.bPrintStats )
    {
        PrintStats(stdout, &stats);
    }

    return 0;

}


int OptimizeVertexMemory( SubMeshData *subGeom )
{

    unsigned int  nFaces    = (unsigned int)  subGeom->numIndices/ 3;
    unsigned int  nVertices = subGeom->numVertices;
    unsigned int* pnIB      = subGeom->indices;


    unsigned int stride3d = 3*sizeof(awd_float64);
    unsigned int stride2d = 2*sizeof(awd_float64);
    unsigned int strideSd = subGeom->bonesPerVertex * sizeof(awd_float64);
    unsigned int strideSi = subGeom->bonesPerVertex * sizeof(unsigned int);




    if (nVertices == 0 || nVertices >= TOOTLE_MAX_VERTICES)
    {
        std::cerr << "TootleOptimizeVertexMemory: nVertices is invalid" << std::endl;
        return 1;
    }

    if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
    {
        std::cerr << "TootleOptimizeVertexMemory: nFaces is invalid" << std::endl;
        return 1;
    }

    unsigned int *tIndices       = new unsigned int[ subGeom->numIndices ];
    char *tVertices      = new char[ nVertices * stride3d ];
    char *tNormals       = NULL;
    char *tTangents      = NULL;
    char *tBinorms       = NULL;
    char *tUVs           = NULL;
    char *tUV2s          = NULL;
    char *tVCs           = NULL;
    char *tSkinWeights   = NULL;
    char *tSkinIndices   = NULL;


    if( subGeom->normals != NULL ){
        tNormals     = new char[ nVertices*stride3d ];
    }
    if( subGeom->tangent != NULL ){
        tTangents    = new char[ nVertices*stride3d ];
    }
    if( subGeom->binorm != NULL ){
        tBinorms     = new char[ nVertices*stride3d ];
    }
    if( subGeom->uvs != NULL ){
        tUVs         = new char[ nVertices*stride2d ];
    }
    if( subGeom->uvs2 != NULL ){
        tUV2s        = new char[ nVertices*stride2d ];
    }
    if( subGeom->colors != NULL ){
        tVCs         = new char[ nVertices*stride3d ];
    }
    if( subGeom->skinWeights != NULL ){
        tSkinWeights = new char[ nVertices * strideSd ];
    }
    if( subGeom->skinIndices != NULL ){
        tSkinIndices = new char[  nVertices * strideSi ];
    }



    // create an array of vertex id map.
    unsigned int* pnVIDRemap = new unsigned int[ nVertices ];

    unsigned int i;
    // mark all vertices map as hasn't been touched/remapped.
    for (i = 0; i < nVertices; i++)
    {
        pnVIDRemap[ i ] = TOOTLE_MAX_VERTICES;
    }

    memcpy(tIndices, pnIB, subGeom->numIndices * sizeof(unsigned int));



    // REMAP THE VERTICES based on the vertex ids in indices array
    unsigned int nVID;
    unsigned int nVIDCount = 0;
    unsigned int nFaces3  = subGeom->numIndices;
    bool bWarning         = true;

    for (i = 0; i < nFaces3; i++)
    {
        nVID = tIndices[ i ];

        // check whether the vertex has been mapped
        if (nVID < nVertices)
        {
            if (pnVIDRemap[ nVID ] == TOOTLE_MAX_VERTICES)
            {
                pnVIDRemap[ nVID ] = nVIDCount++;
            }

            tIndices[ i ] = pnVIDRemap[ nVID ];
        }
        else
        {
            if (bWarning)
            {
                fprintf(stderr, "TootleOptimizeVertexMemory's warning: triangle indices are referencing out-of-bounds vertex buffer.\n");
                bWarning = false;
            }
        }
    }


    // Make sure we map all the vertices.
    // It is possible for some of the vertices not to be referenced by the triangle indices.
    // In this case, we just assign them to the end of the vertex buffer.
    for (i = 0; i < nVertices; i++)
    {
        if (pnVIDRemap[ i ] == TOOTLE_MAX_VERTICES)
        {
            pnVIDRemap[ i ] = nVIDCount++;
        }
    }

    // check the result (make sure we have mapped all the vertices)
    if(nVIDCount != nVertices){
        std::cerr << "TootleOptimizeVertexMemory: map missing" << std::endl;
    }

    for (i = 0; i < nVertices; i++)
    {
        if(pnVIDRemap[ i ] == TOOTLE_MAX_VERTICES){
            std::cerr << "TootleOptimizeVertexMemory: map missing" << std::endl;
        }
    }



    // rearrange the vertex buffer based on the remapping
    const char* verticesBuffer      = (char*) subGeom->vertices;
    const char* normalsBuffer       = (char*) subGeom->normals;
    const char* tangentsBuffer      = (char*) subGeom->tangent;
    const char* binormsBuffer       = (char*) subGeom->binorm;
    const char* uVsBuffer           = (char*) subGeom->uvs;
    const char* uV2sBuffer          = (char*) subGeom->uvs2;
    const char* vCsBuffer           = (char*) subGeom->colors;
    const char* skinWeightsBuffer   = (char*) subGeom->skinWeights;
    const char* skinIndicesBuffer   = (char*) subGeom->skinIndices;


    
    for (i = 0; i < nVertices; i++)
    {
        nVID = pnVIDRemap[ i ];

        memcpy(&tVertices[ nVID * stride3d ], verticesBuffer, stride3d );
        verticesBuffer += stride3d;

        if( normalsBuffer     != NULL ){
            memcpy(&tNormals    [ nVID * stride3d ], normalsBuffer    , stride3d );
            normalsBuffer     += stride3d;
        }
        if( tangentsBuffer    != NULL ){
            memcpy(&tTangents   [ nVID * stride3d ], tangentsBuffer   , stride3d );
            tangentsBuffer    += stride3d;
        }
        if( binormsBuffer     != NULL ){
            memcpy(&tBinorms    [ nVID * stride3d ], binormsBuffer    , stride3d );
            binormsBuffer     += stride3d;
        }
        if( uVsBuffer         != NULL ){
            memcpy(&tUVs        [ nVID * stride2d ], uVsBuffer        , stride2d );
            uVsBuffer         += stride2d;
        }
        if( uV2sBuffer        != NULL ){
            memcpy(&tUV2s       [ nVID * stride2d ], uV2sBuffer       , stride2d );
            uV2sBuffer        += stride2d;
        }
        if( vCsBuffer         != NULL ){
            memcpy(&tVCs        [ nVID * stride3d ], vCsBuffer        , stride3d );
            vCsBuffer         += stride3d;
        }
        if( skinWeightsBuffer != NULL ){
            memcpy(&tSkinWeights[ nVID * strideSd ], skinWeightsBuffer, strideSd );
            skinWeightsBuffer += strideSd;
        }
        if( skinIndicesBuffer != NULL ){
            memcpy(&tSkinIndices[ nVID * strideSi ], skinIndicesBuffer, strideSi );
            skinIndicesBuffer += strideSi;
        }

    }


    memcpy( subGeom->indices, tIndices, subGeom->numIndices * sizeof(unsigned int) );

    memcpy( subGeom->vertices, tVertices, nVertices * stride3d );

    if( normalsBuffer     ){
        memcpy( subGeom->normals      , tNormals    , nVertices*  stride3d );
    }
    if( tangentsBuffer    ){
        memcpy( subGeom->tangent      , tTangents   , nVertices*  stride3d );
    }
    if( binormsBuffer     ){
        memcpy( subGeom->binorm       , tBinorms    , nVertices*  stride3d );
    }
    if( uVsBuffer         ){
        memcpy( subGeom->uvs          , tUVs        , nVertices*  stride2d );
    }
    if( uV2sBuffer        ){
        memcpy( subGeom->uvs2         , tUV2s       , nVertices*  stride2d );
    }
    if( vCsBuffer         ){
        memcpy( subGeom->colors       , tVCs        , nVertices*  stride3d );
    }
    if( skinWeightsBuffer ){
        memcpy( subGeom->skinWeights  , tSkinWeights, nVertices*  strideSd );
    }
    if( skinIndicesBuffer ){
        memcpy( subGeom->skinIndices  , tSkinIndices, nVertices*  strideSi );
    }



    // free temp buffers

    delete[] tIndices ;
    delete[] tVertices;


    if( tNormals     != NULL ) delete[] tNormals    ;
    if( tTangents    != NULL ) delete[] tTangents   ;
    if( tBinorms     != NULL ) delete[] tBinorms    ;
    if( tUVs         != NULL ) delete[] tUVs        ;
    if( tUV2s        != NULL ) delete[] tUV2s       ;
    if( tVCs         != NULL ) delete[] tVCs        ;
    if( tSkinWeights != NULL ) delete[] tSkinWeights;
    if( tSkinIndices != NULL ) delete[] tSkinIndices;

    return 0;
}




//=================================================================================================================================
/// A helper function to print formatted TOOTLE statistics
/// \param f      A file to print the statistics to
/// \param pStats The statistics to be printed
//=================================================================================================================================
void PrintStats(FILE* fp, TootleStats* pStats)
{
    
    fprintf(fp, "#Tootle Stats\n"
            "#Clusters         : %u\n"
            "#CacheIn/Out      : %.3fx (%.3f/%.3f)\n",
            pStats->nClusters,
            pStats->fVCacheIn / pStats->fVCacheOut,
            pStats->fVCacheIn,
            pStats->fVCacheOut);
    
    if (pStats->fMeasureOverdrawTime >= 0)
    {
        fprintf(fp, "#OverdrawIn/Out   : %.3fx (%.3f/%.3f)\n"
                "#OverdrawMaxIn/Out: %.3fx (%.3f/%.3f)\n",
                pStats->fOverdrawIn / pStats->fOverdrawOut,
                pStats->fOverdrawIn,
                pStats->fOverdrawOut,
                pStats->fMaxOverdrawIn / pStats->fMaxOverdrawOut,
                pStats->fMaxOverdrawIn,
                pStats->fMaxOverdrawOut);
    }
    
    fprintf(fp, "\n#Tootle Timings\n");
    
    // print out the timing result if appropriate.
    if (pStats->fOptimizeVCacheTime >= 0)
    {
        fprintf(fp, "#OptimizeVCache               = %.4lf seconds\n", pStats->fOptimizeVCacheTime);
    }
    
    if (pStats->fClusterMeshTime >= 0)
    {
        fprintf(fp, "#ClusterMesh                  = %.4lf seconds\n", pStats->fClusterMeshTime);
    }
    
    if (pStats->fVCacheClustersTime >= 0)
    {
        fprintf(fp, "#VCacheClusters               = %.4lf seconds\n", pStats->fVCacheClustersTime);
    }
    
    if (pStats->fOptimizeVCacheAndClusterMeshTime >= 0)
    {
        fprintf(fp, "#OptimizeVCacheAndClusterMesh = %.4lf seconds\n", pStats->fOptimizeVCacheAndClusterMeshTime);
    }
    
    if (pStats->fOptimizeOverdrawTime >= 0)
    {
        fprintf(fp, "#OptimizeOverdraw             = %.4lf seconds\n", pStats->fOptimizeOverdrawTime);
    }
    
    if (pStats->fTootleOptimizeTime >= 0)
    {
        fprintf(fp, "#TootleOptimize               = %.4lf seconds\n", pStats->fTootleOptimizeTime);
    }
    
    if (pStats->fTootleFastOptimizeTime >= 0)
    {
        fprintf(fp, "#TootleFastOptimize           = %.4lf seconds\n", pStats->fTootleFastOptimizeTime);
    }
    
    if (pStats->fMeasureOverdrawTime >= 0)
    {
        fprintf(fp, "#MeasureOverdraw              = %.4lf seconds\n", pStats->fMeasureOverdrawTime);
    }
    
    if (pStats->fOptimizeVertexMemoryTime >= 0)
    {
        fprintf(fp, "#OptimizeVertexMemory         = %.4lf seconds\n", pStats->fOptimizeVertexMemoryTime);
    }
}



