/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "InvertedDocEndConsumer.h"

namespace Lucene
{
	/// Writes norms.  Each thread X field accumulates the norms for the doc/fields it saw, then the flush method 
	/// below merges all of these together into a single _X.nrm file.
	class LPPAPI NormsWriter : public InvertedDocEndConsumer
	{
	public:
		NormsWriter();
		virtual ~NormsWriter();
		
		LUCENE_CLASS(NormsWriter);
			
	protected:
		FieldInfosPtr fieldInfos;
	
	public:
		virtual InvertedDocEndConsumerPerThreadPtr addThread(DocInverterPerThreadPtr docInverterPerThread);
		virtual void abort();
		
		// We only write the _X.nrm file at flush
		virtual void files(HashSet<String> files);
		
		virtual void setFieldInfos(FieldInfosPtr fieldInfos);
		
		/// Produce _X.nrm if any document had a field with norms not disabled
		virtual void flush(MapInvertedDocEndConsumerPerThreadCollectionInvertedDocEndConsumerPerField threadsAndFields, SegmentWriteStatePtr state);
		virtual void closeDocStore(SegmentWriteStatePtr state);
	
	protected:
		static uint8_t getDefaultNorm();
	};
}
