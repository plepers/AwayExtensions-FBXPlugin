//
//  NodeExporter.h
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 04/11/2014.
//
//

#ifndef __FbxAwdExporter__NodeExporter__
#define __FbxAwdExporter__NodeExporter__

#include <awd/awd.h>
#include <fbxsdk.h>

#include "ExportContext.h"
//This class is a custom writer.
//The writer provide you the ability to write out node hierarchy to a custom file format.



class ExporterProvider;

class NodeExporter
{
public:
    
    void setup( ExportContext *context, ExporterProvider *ep );
    void release();
    
    virtual bool isHandleObject( FbxObject* ) = 0;
   
    virtual void doExport( FbxObject* ) = 0;
    
protected:
    
    ExportContext       *mContext;
    ExporterProvider    *mExporters;
};



//
// ExporterProvider
// Manage a list of node exporter
// retreive the correct NodeExporter for
// each nodes
//

class ExporterLinkedItem
{
public:
    ExporterLinkedItem( NodeExporter* exporter );
    ~ExporterLinkedItem();
    
    ExporterLinkedItem* next();
    void setNext( ExporterLinkedItem*  );
    
    NodeExporter* getExporter();
    
    
private:
    
    NodeExporter* mExporter;
    ExporterLinkedItem* mNext;
    
};


class ExporterProvider
{
public:
    ExporterProvider( NodeExporter* pDefault );
    ~ExporterProvider();
    
    void addExporter( NodeExporter* );
    
    NodeExporter* findExporter( FbxObject* pObj );
    NodeExporter* getDefaultExporter();
    
private:
    ExporterLinkedItem 	*mHead;
    ExporterLinkedItem 	*mTail;
    NodeExporter		*mDefault;
};





#endif /* defined(__FbxAwdExporter__NodeExporter__) */
