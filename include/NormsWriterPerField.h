/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "InvertedDocEndConsumerPerField.h"

namespace Lucene
{
	/// Taps into DocInverter, as an InvertedDocEndConsumer, which is called at the end of inverting each field.  
	/// We just look at the length for the field (docState.length) and record the norm.
	class LPPAPI NormsWriterPerField : public InvertedDocEndConsumerPerField
	{
	public:
		NormsWriterPerField(DocInverterPerFieldPtr docInverterPerField, NormsWriterPerThreadPtr perThread, FieldInfoPtr fieldInfo);
		virtual ~NormsWriterPerField();
		
		LUCENE_CLASS(NormsWriterPerField);
			
	public:
		NormsWriterPerThreadWeakPtr _perThread;
		FieldInfoPtr fieldInfo;
		DocStatePtr docState;
		
		// Holds all docID/norm pairs we've seen
		Collection<int32_t> docIDs;
		ByteArray norms;
		int32_t upto;
		
		FieldInvertStatePtr fieldState;
	
	public:
		void reset();
		virtual void abort();
		
		/// Compare two objects
		virtual int32_t compareTo(LuceneObjectPtr other);
		
		virtual void finish();
	};
}
