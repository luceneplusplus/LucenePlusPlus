/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FORMATPOSTINGSPOSITIONSWRITER_H
#define FORMATPOSTINGSPOSITIONSWRITER_H

#include "FormatPostingsPositionsConsumer.h"

namespace Lucene
{
    class FormatPostingsPositionsWriter : public FormatPostingsPositionsConsumer
    {
    public:
        FormatPostingsPositionsWriter(SegmentWriteStatePtr state, FormatPostingsDocsWriterPtr parent);
        virtual ~FormatPostingsPositionsWriter();

        LUCENE_CLASS(FormatPostingsPositionsWriter);

    public:
        FormatPostingsDocsWriterPtr parent;
        IndexOutputPtr out;

        bool omitTermFreqAndPositions;
        bool storePayloads;
        int32_t lastPayloadLength;

        int32_t lastPosition;

    protected:
        virtual void mark_members(gc* gc) const
        {
            gc->mark(parent);
            gc->mark(out);
            FormatPostingsPositionsConsumer::mark_members(gc);
        }

    public:
        /// Add a new position & payload
        virtual void addPosition(int32_t position, ByteArray payload, int32_t payloadOffset, int32_t payloadLength);

        void setField(FieldInfoPtr fieldInfo);

        /// Called when we are done adding positions & payloads
        virtual void finish();

        void close();
    };
}

#endif
