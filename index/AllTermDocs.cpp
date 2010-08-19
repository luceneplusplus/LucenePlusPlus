/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AllTermDocs.h"
#include "SegmentReader.h"
#include "BitVector.h"

namespace Lucene
{
    AllTermDocs::AllTermDocs(SegmentReaderPtr parent)
    {
        {
            SyncLock parentLock(parent);
            this->_deletedDocs = parent->deletedDocs;
        }
        this->maxDoc = parent->maxDoc();
        this->_doc = -1;
    }
    
    AllTermDocs::~AllTermDocs()
    {
    }
    
    void AllTermDocs::seek(TermPtr term)
    {
        if (!term)
            _doc = -1;
        else
            boost::throw_exception(UnsupportedOperationException());
    }
    
    void AllTermDocs::seek(TermEnumPtr termEnum)
    {
        boost::throw_exception(UnsupportedOperationException());
    }
    
    int32_t AllTermDocs::doc()
    {
        return _doc;
    }
    
    int32_t AllTermDocs::freq()
    {
        return 1;
    }
    
    bool AllTermDocs::next()
    {
        return skipTo(_doc + 1);
    }
    
    int32_t AllTermDocs::read(Collection<int32_t> docs, Collection<int32_t> freqs)
    {
        int32_t length = docs.size();
        int32_t i = 0;
        BitVectorPtr deletedDocs(_deletedDocs.lock());
        while (i < length && _doc < maxDoc)
        {
            if (!deletedDocs || !deletedDocs->get(_doc))
            {
                docs[i] = _doc;
                freqs[i] = 1;
                ++i;
            }
            ++_doc;
        }
        return i;
    }
    
    bool AllTermDocs::skipTo(int32_t target)
    {
        _doc = target;
        BitVectorPtr deletedDocs(_deletedDocs.lock());
        while (_doc < maxDoc)
        {
            if (!deletedDocs || !deletedDocs->get(_doc))
                return true;
            ++_doc;
        }
        return false;
    }
    
    void AllTermDocs::close()
    {
    }
}
