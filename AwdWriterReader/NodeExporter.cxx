//
//  NodeExporter.cpp
//  FbxAwdExporter
//
//  Created by Pierre Lepers on 04/11/2014.
//
//

#include "NodeExporter.h"



void NodeExporter::setup( ExportContext *context, ExporterProvider *provider )
{
    mContext = context;
    mExporters = provider;
}


void NodeExporter::release( )
{
    mContext = NULL;
}




// ================
// ExporterProvider


ExporterLinkedItem::ExporterLinkedItem(NodeExporter *exporter )
{
    mExporter = exporter;
    mNext = NULL;
}

ExporterLinkedItem::~ExporterLinkedItem()
{
    mExporter = NULL;
    mNext = NULL;
}


ExporterLinkedItem* ExporterLinkedItem::next()
{
    return mNext;
}

void ExporterLinkedItem::setNext( ExporterLinkedItem* item )
{
    if( mNext ) {
        item->setNext( mNext );
    }
    mNext = item;
}

NodeExporter* ExporterLinkedItem::getExporter()
{
    return mExporter;
}






ExporterProvider::ExporterProvider( NodeExporter* pDefault )
{
    mDefault = pDefault;
    mHead = NULL;
    mTail = NULL;
}

ExporterProvider::~ExporterProvider( )
{
    ExporterLinkedItem *cur = mHead;
    while (cur) {
        ExporterLinkedItem *next = cur->next();
        delete cur;
        cur = next;
    }
    mHead = mTail = NULL;
}

void ExporterProvider::addExporter(NodeExporter *exporter)
{
    ExporterLinkedItem *item = new ExporterLinkedItem( exporter );
    if( mTail ) {
        mTail->setNext(item);
    } else {
        mHead = mTail = item;
    }
}

NodeExporter* ExporterProvider::findExporter( FbxObject* pObj )
{
    ExporterLinkedItem *item = mHead;
    
    while( item ) {
        
        if( item->getExporter()->isHandleObject( pObj ) ){
            return item->getExporter();
        }
        
        item = item->next();
    }
    
    // no suitable exporter found
    return NULL;
}



NodeExporter* ExporterProvider::getDefaultExporter()
{
    return mDefault;
}

