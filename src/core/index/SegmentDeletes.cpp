/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SegmentDeletes.h"
#include "AtomicLong.h"
#include "StringUtils.h"
#include "Term.h"
#include "Query.h"

namespace Lucene
{
    /// Rough logic: HashMap has an array[Entry] with varying load factor (say 2 * POINTER).
    /// Entry is object with Term key, Integer val, int hash, Entry next 
    /// (OBJ_HEADER + 3*POINTER + INT).  Term is object with String field and String text 
    /// (OBJ_HEADER + 2*POINTER).
    /// We don't count Term's field since it's interned. Term's text is 
    /// String (OBJ_HEADER + 4*INT + POINTER + OBJ_HEADER + string.length*CHAR).  
    /// Integer is OBJ_HEADER + INT.
    const int32_t SegmentDeletes::BYTES_PER_DEL_TERM = 8 * sizeof(LuceneObjectPtr) + 5 * 8 + 6 * sizeof(int32_t);
    
    /// Rough logic: del docIDs are List<Integer>.  Say list allocates ~2X size (2*POINTER).
    /// Integer is OBJ_HEADER + int
    const int32_t SegmentDeletes::BYTES_PER_DEL_DOCID = 2 * sizeof(LuceneObjectPtr) + 8 + sizeof(int32_t);
    
    /// Rough logic: HashMap has an array[Entry] with varying load factor (say 2 * POINTER).
    /// Entry is object with Query key, Integer val, int hash, 
    /// Entry next (OBJ_HEADER + 3*POINTER + INT).  Query we often undercount (say 24 bytes).
    /// Integer is OBJ_HEADER + INT.
    const int32_t SegmentDeletes::BYTES_PER_DEL_QUERY = 5 * sizeof(LuceneObjectPtr) + 2 * 8 + 2 * sizeof(int32_t) + 24;
    
    const bool SegmentDeletes::VERBOSE_DELETES = false;
    
    SegmentDeletes::SegmentDeletes()
    {
        numTermDeletes = newLucene<AtomicLong>();
        terms = SortedMapTermInt::newInstance();
        queries = MapQueryInt::newInstance();
        docIDs = Collection<int32_t>::newInstance();
        bytesUsed = newLucene<AtomicLong>();
    }
    
    SegmentDeletes::~SegmentDeletes()
    {
    }
    
    String SegmentDeletes::toString()
    {
        if (VERBOSE_DELETES)
        {
            return L"SegmentDeletes [numTerms=" + StringUtils::toString(numTermDeletes->get()) + 
                   L", terms=" + StringUtils::toString(terms.size()) +
                   L", queries=" + StringUtils::toString(queries.size()) + 
                   L", docIDs=" + StringUtils::toString(docIDs.size()) + 
                   L", bytesUsed=" + StringUtils::toString(bytesUsed->get()) + L"]";
        }
        else
        {
            StringStream buffer;
            if (numTermDeletes->get() != 0)
                buffer << L" " << numTermDeletes->get() << L" deleted terms (unique count=" << terms.size() << L")";
            if (!queries.empty())
                buffer << L" " << queries.size() << L" deleted queries";
            if (!docIDs.empty())
                buffer << L" " << docIDs.size() << L" deleted docIDs";
            if (bytesUsed->get() != 0)
                buffer << L" bytesUsed=" << bytesUsed->get();
            return buffer.str();
        }
    }
    
    void SegmentDeletes::update(SegmentDeletesPtr in, bool noLimit)
    {
        numTermDeletes->addAndGet(in->numTermDeletes->get());
        for (SortedMapTermInt::iterator ent = in->terms.begin(); ent != in->terms.end(); ++ent)
        {
            TermPtr term(ent->first);
            if (!terms.contains(term))
            {
                // only incr bytesUsed if this term wasn't already buffered:
                bytesUsed->addAndGet(BYTES_PER_DEL_TERM);
            }
            int32_t limit = noLimit ? INT_MAX : ent->second;
            terms.put(term, limit);
        }

        for (MapQueryInt::iterator ent = in->queries.begin(); ent != in->queries.end(); ++ent)
        {
            QueryPtr query(ent->first);
            if (!queries.contains(query))
            {
                // only incr bytesUsed if this query wasn't already buffered:
                bytesUsed->addAndGet(BYTES_PER_DEL_QUERY);
            }
            int32_t limit = noLimit ? INT_MAX : ent->second;
            queries.put(query, limit);
        }

        // docIDs never move across segments and the docIDs should already be cleared
    }
    
    void SegmentDeletes::addQuery(QueryPtr query, int32_t docIDUpto)
    {
        // increment bytes used only if the query wasn't added so far.
        if (queries.contains(query))
            bytesUsed->addAndGet(BYTES_PER_DEL_QUERY);
        queries.put(query, docIDUpto);
    }
    
    void SegmentDeletes::addDocID(int32_t docID)
    {
        docIDs.add(docID);
        bytesUsed->addAndGet(BYTES_PER_DEL_DOCID);
    }
    
    void SegmentDeletes::addTerm(TermPtr term, int32_t docIDUpto)
    {
        bool termExists = terms.contains(term);
        if (termExists && docIDUpto < terms[term])
        {
            // Only record the new number if it's greater than the current one.  This is important 
            // because if multiple threads are replacing the same doc at nearly the same time, it's 
            // possible that one thread that got a higher docID is scheduled before the other threads.
            // If we blindly replace than we can get double-doc in the segment.
            return;
        }

        terms.put(term, docIDUpto);
        numTermDeletes->incrementAndGet();
        if (!termExists)
            bytesUsed->addAndGet(BYTES_PER_DEL_TERM + term->_text.length() * sizeof(wchar_t));
    }
    
    void SegmentDeletes::clear()
    {
        terms.clear();
        queries.clear();
        docIDs.clear();
        numTermDeletes->set(0);
        bytesUsed->set(0);
    }
    
    void SegmentDeletes::clearDocIDs()
    {
        bytesUsed->addAndGet(-docIDs.size() * BYTES_PER_DEL_DOCID);
        docIDs.clear();
    }
    
    bool SegmentDeletes::any()
    {
        return (!terms.empty() || !docIDs.empty() || !queries.empty());
    }
}
