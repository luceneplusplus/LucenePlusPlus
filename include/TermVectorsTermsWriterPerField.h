/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TERMVECTORSTERMSWRITERPERFIELD_H
#define TERMVECTORSTERMSWRITERPERFIELD_H

#include "TermsHashConsumerPerField.h"

namespace Lucene
{
    class LPPAPI TermVectorsTermsWriterPerField : public TermsHashConsumerPerField
    {
    public:
        TermVectorsTermsWriterPerField(TermsHashPerFieldPtr termsHashPerField, TermVectorsTermsWriterPerThreadPtr perThread, FieldInfoPtr fieldInfo);
        virtual ~TermVectorsTermsWriterPerField();
        
        LUCENE_CLASS(TermVectorsTermsWriterPerField);
            
    public:
        TermVectorsTermsWriterPerThreadWeakPtr _perThread;
        TermsHashPerFieldWeakPtr _termsHashPerField;
        TermVectorsTermsWriterWeakPtr _termsWriter;
        FieldInfoPtr fieldInfo;
        DocStateWeakPtr _docState;
        FieldInvertStateWeakPtr _fieldState;
        
        bool doVectors;
        bool doVectorPositions;
        bool doVectorOffsets;
        
        int32_t maxNumPostings;
        OffsetAttributePtr offsetAttribute;
    
    public:
        virtual int32_t getStreamCount();
        virtual bool start(Collection<FieldablePtr> fields, int32_t count);
        virtual void abort();
        
        /// Called once per field per document if term vectors are enabled, to write the vectors to RAMOutputStream, 
        /// which is then quickly flushed to the real term vectors files in the Directory.
        virtual void finish();
        
        void shrinkHash();
        
        virtual void start(FieldablePtr field);
        virtual void newTerm(RawPostingListPtr p0);
        virtual void addTerm(RawPostingListPtr p0);
        virtual void skippingLongTerm();
    };
}

#endif
