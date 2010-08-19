/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DocumentsWriter.h"

namespace Lucene
{
	/// This is a DocFieldConsumer that writes stored fields.
	class LPPAPI StoredFieldsWriter : public LuceneObject
	{
	public:
		StoredFieldsWriter(DocumentsWriterPtr docWriter, FieldInfosPtr fieldInfos);
		virtual ~StoredFieldsWriter();
		
		LUCENE_CLASS(StoredFieldsWriter);
			
	public:
		FieldsWriterPtr fieldsWriter;
		DocumentsWriterWeakPtr _docWriter;
		FieldInfosPtr fieldInfos;
		int32_t lastDocID;

		Collection<StoredFieldsWriterPerDocPtr> docFreeList;
		int32_t freeCount;
		int32_t allocCount;
		
	public:
		StoredFieldsWriterPerThreadPtr addThread(DocStatePtr docState);
		void flush(SegmentWriteStatePtr state);
		void closeDocStore(SegmentWriteStatePtr state);
		StoredFieldsWriterPerDocPtr getPerDoc();
		void abort();
		
		/// Fills in any hole in the docIDs
		void fill(int32_t docID);
		
		void finishDocument(StoredFieldsWriterPerDocPtr perDoc);
		bool freeRAM();
		void free(StoredFieldsWriterPerDocPtr perDoc);
		
	protected:
		void initFieldsWriter();
	};
	
	class LPPAPI StoredFieldsWriterPerDoc : public DocWriter
	{
	public:
		StoredFieldsWriterPerDoc(StoredFieldsWriterPtr fieldsWriter);
		virtual ~StoredFieldsWriterPerDoc();
		
		LUCENE_CLASS(StoredFieldsWriterPerDoc);
			
	protected:
		StoredFieldsWriterWeakPtr _fieldsWriter;
	
	public:
		RAMOutputStreamPtr fdt;
		int32_t numStoredFields;
	
	public:
		void reset();
		virtual void abort();
		virtual int64_t sizeInBytes();
		virtual void finish();
	};
}
