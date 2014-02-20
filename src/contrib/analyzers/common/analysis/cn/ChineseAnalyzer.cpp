/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ChineseAnalyzer.h"
#include "ChineseTokenizer.h"
#include "ChineseFilter.h"

namespace Lucene {

ChineseAnalyzer::~ChineseAnalyzer() {
}

TokenStreamPtr ChineseAnalyzer::tokenStream(const String& fieldName, const ReaderPtr& reader) {
    TokenStreamPtr result = newLucene<ChineseTokenizer>(reader);
    result = newLucene<ChineseFilter>(result);
    return result;
}

TokenStreamPtr ChineseAnalyzer::reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
    ChineseAnalyzerSavedStreamsPtr streams(boost::dynamic_pointer_cast<ChineseAnalyzerSavedStreams>(getPreviousTokenStream()));
    if (!streams) {
        streams = newLucene<ChineseAnalyzerSavedStreams>();
        streams->source = newLucene<ChineseTokenizer>(reader);
        streams->result = newLucene<ChineseFilter>(streams->source);
        setPreviousTokenStream(streams);
    } else {
        streams->source->reset(reader);
    }
    return streams->result;
}

ChineseAnalyzerSavedStreams::~ChineseAnalyzerSavedStreams() {
}

}
