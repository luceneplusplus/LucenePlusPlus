/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FORMATPOSTINGSTERMSWRITER_H
#define FORMATPOSTINGSTERMSWRITER_H

#include "FormatPostingsTermsConsumer.h"

namespace Lucene
{
    class FormatPostingsTermsWriter : public FormatPostingsTermsConsumer
    {
    public:
        FormatPostingsTermsWriter(SegmentWriteStatePtr state, FormatPostingsFieldsWriterPtr parent);
        virtual ~FormatPostingsTermsWriter();

        LUCENE_CLASS(FormatPostingsTermsWriter);

    public:
        FormatPostingsFieldsWriterPtr parent;
        SegmentWriteStatePtr state;
        FormatPostingsDocsWriterPtr docsWriter;
        TermInfosWriterPtr termsOut;
        FieldInfoPtr fieldInfo;

        CharArray currentTerm;
        int32_t currentTermStart;

        int64_t freqStart;
        int64_t proxStart;

    protected:
        virtual void mark_members(gc* gc) const
        {
            gc->mark(parent);
            gc->mark(state);
            gc->mark(docsWriter);
            gc->mark(termsOut);
            gc->mark(fieldInfo);
            gc->mark(currentTerm);
            FormatPostingsTermsConsumer::mark_members(gc);
        }

    public:
        virtual void initialize();

        void setField(FieldInfoPtr fieldInfo);

        /// Adds a new term in this field
        virtual FormatPostingsDocsConsumerPtr addTerm(CharArray text, int32_t start);

        /// Called when we are done adding terms to this field
        virtual void finish();

        void close();
    };
}

#endif
