/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "TermDocs.h"

namespace Lucene
{
	class LPPAPI AllTermDocs : public TermDocs, public LuceneObject
	{
	public:
		AllTermDocs(SegmentReaderPtr parent);
		virtual ~AllTermDocs();
		
		LUCENE_CLASS(AllTermDocs);
	
	protected:
		BitVectorWeakPtr _deletedDocs;
		int32_t maxDoc;
		int32_t _doc;
		
	public:
		/// Sets this to the data for a term.
		virtual void seek(TermPtr term);
		
		/// Sets this to the data for the current term in a {@link TermEnum}.
		virtual void seek(TermEnumPtr termEnum);
		
		/// Returns the current document number.
		virtual int32_t doc();
		
		/// Returns the frequency of the term within the current document.
		virtual int32_t freq();
		
		/// Moves to the next pair in the enumeration.
		virtual bool next();
		
		/// Attempts to read multiple entries from the enumeration, up to length of docs.
		virtual int32_t read(Collection<int32_t> docs, Collection<int32_t> freqs);
		
		/// Skips entries to the first beyond the current whose document number is greater than or 
		/// equal to target.
		virtual bool skipTo(int32_t target);
		
		/// Frees associated resources.
		virtual void close();
	};
}
