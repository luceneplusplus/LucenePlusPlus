/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "CJKAnalyzer.h"
#include "CJKTokenizer.h"
#include "StopFilter.h"

namespace Lucene {

const wchar_t* CJKAnalyzer::_STOP_WORDS[] = {
    L"a", L"and", L"are", L"as", L"at", L"be",
    L"but", L"by", L"for", L"if", L"in", L"into",
    L"is", L"it", L"no", L"not", L"of", L"on",
    L"or", L"s", L"such", L"t", L"that", L"the",
    L"their", L"then", L"there", L"these",
    L"they", L"this", L"to", L"was", L"will",
    L"with", L"", L"www"
};

CJKAnalyzer::CJKAnalyzer(LuceneVersion::Version matchVersion) {
    this->stoptable = getDefaultStopSet();
    this->matchVersion = matchVersion;
}

CJKAnalyzer::CJKAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords) {
    this->stoptable = stopwords;
    this->matchVersion = matchVersion;
}

CJKAnalyzer::~CJKAnalyzer() {
}

const HashSet<String> CJKAnalyzer::getDefaultStopSet() {
    static HashSet<String> stopSet;
    if (!stopSet) {
        stopSet = HashSet<String>::newInstance(_STOP_WORDS, _STOP_WORDS + SIZEOF_ARRAY(_STOP_WORDS));
    }
    return stopSet;
}

TokenStreamPtr CJKAnalyzer::tokenStream(const String& fieldName, const ReaderPtr& reader) {
    return newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), newLucene<CJKTokenizer>(reader), stoptable);
}

TokenStreamPtr CJKAnalyzer::reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
    CJKAnalyzerSavedStreamsPtr streams(boost::dynamic_pointer_cast<CJKAnalyzerSavedStreams>(getPreviousTokenStream()));
    if (!streams) {
        streams = newLucene<CJKAnalyzerSavedStreams>();
        streams->source = newLucene<CJKTokenizer>(reader);
        streams->result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), streams->source, stoptable);
        setPreviousTokenStream(streams);
    } else {
        streams->source->reset(reader);
    }
    return streams->result;
}

CJKAnalyzerSavedStreams::~CJKAnalyzerSavedStreams() {
}

}
