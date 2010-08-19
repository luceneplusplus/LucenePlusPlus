/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DocFieldConsumerPerThread.h"

namespace Lucene
{
	class LPPAPI DocFieldConsumersPerThread : public DocFieldConsumerPerThread
	{
	public:
		DocFieldConsumersPerThread(DocFieldProcessorPerThreadPtr docFieldProcessorPerThread, DocFieldConsumersPtr parent, 
								   DocFieldConsumerPerThreadPtr one, DocFieldConsumerPerThreadPtr two);
		virtual ~DocFieldConsumersPerThread();
		
		LUCENE_CLASS(DocFieldConsumersPerThread);
			
	public:
		DocFieldConsumerPerThreadPtr one;
		DocFieldConsumerPerThreadPtr two;
		DocFieldConsumersWeakPtr _parent;
		DocStatePtr docState;
	
	public:
		virtual void startDocument();
		virtual void abort();
		virtual DocWriterPtr finishDocument();
		virtual DocFieldConsumerPerFieldPtr addField(FieldInfoPtr fi);		
	};
}
