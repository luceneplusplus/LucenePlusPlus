/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "SnowballAnalyzer.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "LowerCaseFilter.h"
#include "StopFilter.h"
#include "SnowballFilter.h"

namespace Lucene {

SnowballAnalyzer::SnowballAnalyzer(LuceneVersion::Version matchVersion, const String& name) {
    this->matchVersion = matchVersion;
    this->name = name;
}

SnowballAnalyzer::SnowballAnalyzer(LuceneVersion::Version matchVersion, const String& name, HashSet<String> stopwords) {
    this->stopSet = stopwords;
    this->matchVersion = matchVersion;
    this->name = name;
}

SnowballAnalyzer::~SnowballAnalyzer() {
}

TokenStreamPtr SnowballAnalyzer::tokenStream(const String& fieldName, const ReaderPtr& reader) {
    TokenStreamPtr result = newLucene<StandardTokenizer>(matchVersion, reader);
    result = newLucene<StandardFilter>(result);
    result = newLucene<LowerCaseFilter>(result);
    if (stopSet) {
        result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), result, stopSet);
    }
    result = newLucene<SnowballFilter>(result, name);
    return result;
}

TokenStreamPtr SnowballAnalyzer::reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
    SnowballAnalyzerSavedStreamsPtr streams(boost::dynamic_pointer_cast<SnowballAnalyzerSavedStreams>(getPreviousTokenStream()));
    if (!streams) {
        streams = newLucene<SnowballAnalyzerSavedStreams>();
        streams->source = newLucene<StandardTokenizer>(matchVersion, reader);
        streams->result = newLucene<StandardFilter>(streams->source);
        streams->result = newLucene<LowerCaseFilter>(streams->result);
        if (stopSet) {
            streams->result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), streams->result, stopSet);
        }
        streams->result = newLucene<SnowballFilter>(streams->result, name);
        setPreviousTokenStream(streams);
    } else {
        streams->source->reset(reader);
    }
    return streams->result;
}

SnowballAnalyzerSavedStreams::~SnowballAnalyzerSavedStreams() {
}

}
