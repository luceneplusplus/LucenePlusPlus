/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	class LPPAPI InvertedDocEndConsumer : public LuceneObject
	{
	public:
		virtual ~InvertedDocEndConsumer();
		
		LUCENE_CLASS(InvertedDocEndConsumer);
	
	public:
		virtual InvertedDocEndConsumerPerThreadPtr addThread(DocInverterPerThreadPtr docInverterPerThread) = 0;
		virtual void flush(MapInvertedDocEndConsumerPerThreadCollectionInvertedDocEndConsumerPerField threadsAndFields, SegmentWriteStatePtr state) = 0;
		virtual void closeDocStore(SegmentWriteStatePtr state) = 0;
		virtual void abort() = 0;
		virtual void setFieldInfos(FieldInfosPtr fieldInfos) = 0;
	};
}
