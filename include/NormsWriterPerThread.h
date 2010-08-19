/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "InvertedDocEndConsumerPerThread.h"

namespace Lucene
{
	class LPPAPI NormsWriterPerThread : public InvertedDocEndConsumerPerThread
	{
	public:
		NormsWriterPerThread(DocInverterPerThreadPtr docInverterPerThread, NormsWriterPtr normsWriter);
		virtual ~NormsWriterPerThread();
		
		LUCENE_CLASS(NormsWriterPerThread);
			
	public:
		NormsWriterWeakPtr _normsWriter;
		DocStatePtr docState;
	
	public:
		virtual InvertedDocEndConsumerPerFieldPtr addField(DocInverterPerFieldPtr docInverterPerField, FieldInfoPtr fieldInfo);
		virtual void abort();
		virtual void startDocument();
		virtual void finishDocument();
		
		bool freeRAM();
	};
}
