//
//  tootle.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 11/06/2016.
//
//

#ifndef FbxAwdExporter_tootle_h
#define FbxAwdExporter_tootle_h


#include <iostream>
#include "tootlelib.h"
#include "GeomData.h"
#include <awd/awd.h>

//=================================================================================================================================
/// Enumeration for the choice of test cases for tootle.
//=================================================================================================================================
enum TootleAlgorithm
{
    NA_TOOTLE_ALGORITHM,                // Default invalid choice.
    TOOTLE_VCACHE_ONLY,                 // Only perform vertex cache optimization.
    TOOTLE_CLUSTER_VCACHE_OVERDRAW,     // Call the clustering, optimize vertex cache and overdraw individually.
    TOOTLE_FAST_VCACHECLUSTER_OVERDRAW, // Call the functions to optimize vertex cache and overdraw individually.  This is using
    //  the algorithm from SIGGRAPH 2007.
    TOOTLE_OPTIMIZE,                    // Call a single function to optimize vertex cache, cluster and overdraw.
    TOOTLE_FAST_OPTIMIZE                // Call a single function to optimize vertex cache, cluster and overdraw using
    //  a fast algorithm from SIGGRAPH 2007.
};

//=================================================================================================================================
/// A simple structure to store the settings for this sample app
//=================================================================================================================================
struct TootleSettings
{
    const char*           pMeshName ;
    const char*           pViewpointName ;
    unsigned int          nClustering ;
    unsigned int          nCacheSize;
    TootleFaceWinding     eWinding;
    TootleAlgorithm       algorithmChoice;         // five different types of algorithm to test Tootle
    TootleVCacheOptimizer eVCacheOptimizer;        // the choice for vertex cache optimization algorithm, it can be either
    //  TOOTLE_VCACHE_AUTO, TOOTLE_VCACHE_LSTRIPS, TOOTLE_VCACHE_DIRECT3D or
    //  TOOTLE_VCACHE_TIPSY.
    bool                  bOptimizeVertexMemory;   // true if you want to optimize vertex memory location, false to skip
    bool                  bMeasureOverdraw;        // true if you want to measure overdraw, false to skip
    bool                  bPrintStats;
};

//=================================================================================================================================
/// A simple structure to hold Tootle statistics
//=================================================================================================================================
struct TootleStats
{
    unsigned int nClusters;
    float        fVCacheIn;
    float        fVCacheOut;
    float        fOverdrawIn;
    float        fOverdrawOut;
    float        fMaxOverdrawIn;
    float        fMaxOverdrawOut;
    double       fOptimizeVCacheTime;
    double       fClusterMeshTime;
    double       fOptimizeOverdrawTime;
    double       fVCacheClustersTime;
    double       fOptimizeVCacheAndClusterMeshTime;
    double       fTootleOptimizeTime;
    double       fTootleFastOptimizeTime;
    double       fMeasureOverdrawTime;
    double       fOptimizeVertexMemoryTime;
};

const float INVALID_TIME = -1;


void DisplayTootleErrorMessage(TootleResult eResult);

int ProcessTootleSubMeshData( SubMeshData *subGeom, TootleSettings& settings );
int OptimizeVertexMemory( SubMeshData *subGeom );


void PrintStats(FILE* fp, TootleStats* pStats);

#endif
