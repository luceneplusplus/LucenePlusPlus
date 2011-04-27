/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "FreqProxFieldMergeState.h"
#include "FreqProxTermsWriterPerField.h"
#include "_FreqProxTermsWriterPerField.h"
#include "FreqProxTermsWriterPerThread.h"
#include "FreqProxTermsWriter.h"
#include "TermsHashPerThread.h"
#include "TermsHashPerField.h"
#include "ByteSliceReader.h"
#include "DocumentsWriter.h"
#include "CharBlockPool.h"
#include "FieldInfo.h"
#include "MiscUtils.h"

namespace Lucene
{
    FreqProxFieldMergeState::FreqProxFieldMergeState(FreqProxTermsWriterPerFieldPtr field)
    {
        this->numPostings = 0;
        this->textOffset = 0;
        this->docID = 0;
        this->termFreq = 0;
        this->postingUpto = -1;
        this->currentTermID = 0;
        this->freq = newLucene<ByteSliceReader>();
        this->prox = newLucene<ByteSliceReader>();
        
        this->field = field;
        this->charPool = TermsHashPerThreadPtr(FreqProxTermsWriterPerThreadPtr(field->_perThread)->_termsHashPerThread)->charPool;
        
        TermsHashPerFieldPtr termsHashPerField(field->_termsHashPerField);
        this->numPostings = termsHashPerField->numPostings;
        this->termIDs = termsHashPerField->sortPostings();
        this->postings = boost::static_pointer_cast<FreqProxPostingsArray>(termsHashPerField->postingsArray);
    }
    
    FreqProxFieldMergeState::~FreqProxFieldMergeState()
    {
    }
    
    bool FreqProxFieldMergeState::nextTerm()
    {
        ++postingUpto;
        if (postingUpto == numPostings)
            return false;
        
        currentTermID = termIDs[postingUpto];
        docID = 0;

        int32_t textStart = postings->textStarts[currentTermID];
        text = charPool->buffers[textStart >> DocumentsWriter::CHAR_BLOCK_SHIFT];
        textOffset = (textStart & DocumentsWriter::CHAR_BLOCK_MASK);

        TermsHashPerFieldPtr termsHashPerField(field->_termsHashPerField);
        termsHashPerField->initReader(freq, currentTermID, 0);
        if (!field->fieldInfo->omitTermFreqAndPositions)
            termsHashPerField->initReader(prox, currentTermID, 1);
        
        // Should always be true
        bool result = nextDoc();
        BOOST_ASSERT(result);
        
        return true;
    }
    
    bool FreqProxFieldMergeState::nextDoc()
    {
        if (freq->eof())
        {
            if (postings->lastDocCodes[currentTermID] != -1)
            {
                // Return last doc
                docID = postings->lastDocIDs[currentTermID];
                if (!field->omitTermFreqAndPositions)
                    termFreq = postings->docFreqs[currentTermID];
                postings->lastDocCodes[currentTermID] = -1;
                return true;
            }
            else
            {
                // EOF
                return false;
            }
        }
        
        int32_t code = freq->readVInt();
        if (field->omitTermFreqAndPositions)
            docID += code;
        else
        {
            docID += MiscUtils::unsignedShift(code, 1);
            if ((code & 1) != 0)
                termFreq = 1;
            else
                termFreq = freq->readVInt();
        }
        
        BOOST_ASSERT(docID != postings->lastDocIDs[currentTermID]);
        
        return true;
    }
}
