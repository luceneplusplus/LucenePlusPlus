/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FREQPROXTERMSWRITERPERFIELD_H
#define FREQPROXTERMSWRITERPERFIELD_H

#include "TermsHashConsumerPerField.h"

namespace Lucene
{
    class FreqProxTermsWriterPerField : public TermsHashConsumerPerField
    {
    public:
        FreqProxTermsWriterPerField(TermsHashPerFieldPtr termsHashPerField, FreqProxTermsWriterPerThreadPtr perThread, FieldInfoPtr fieldInfo);
        virtual ~FreqProxTermsWriterPerField();

        LUCENE_CLASS(FreqProxTermsWriterPerField);

    public:
        FreqProxTermsWriterPerThreadPtr perThread;
        TermsHashPerFieldPtr termsHashPerField;
        FieldInfoPtr fieldInfo;
        DocStatePtr docState;
        FieldInvertStatePtr fieldState;
        bool omitTermFreqAndPositions;
        PayloadAttributePtr payloadAttribute;
        bool hasPayloads;

    public:
        virtual int32_t getStreamCount();
        virtual void finish();
        virtual void skippingLongTerm();
        virtual int32_t compareTo(LuceneObjectPtr other);
        void reset();
        virtual bool start(Collection<FieldablePtr> fields, int32_t count);
        virtual void start(FieldablePtr field);
        void writeProx(FreqProxTermsWriterPostingListPtr p, int32_t proxCode);
        virtual void newTerm(RawPostingListPtr p);
        virtual void addTerm(RawPostingListPtr p);
        void abort();
    };
}

#endif
