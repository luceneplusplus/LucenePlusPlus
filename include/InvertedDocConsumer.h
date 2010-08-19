/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI InvertedDocConsumer : public LuceneObject
	{
	public:
		virtual ~InvertedDocConsumer();
		
		LUCENE_CLASS(InvertedDocConsumer);
	
	public:
		FieldInfosPtr fieldInfos;
	
	public:
		/// Add a new thread
		virtual InvertedDocConsumerPerThreadPtr addThread(DocInverterPerThreadPtr docInverterPerThread) = 0;
		
		/// Abort (called after hitting AbortException)
		virtual void abort() = 0;
		
		/// Flush a new segment
		virtual void flush(MapInvertedDocConsumerPerThreadCollectionInvertedDocConsumerPerField threadsAndFields, SegmentWriteStatePtr state) = 0;
		
		/// Close doc stores
		virtual void closeDocStore(SegmentWriteStatePtr state) = 0;
		
		/// Attempt to free RAM, returning true if any RAM was freed
		virtual bool freeRAM() = 0;
		
		virtual void setFieldInfos(FieldInfosPtr fieldInfos);
	};
}
