/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "GermanAnalyzer.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "LowerCaseFilter.h"
#include "StopFilter.h"
#include "GermanStemFilter.h"

namespace Lucene {

const wchar_t* GermanAnalyzer::_GERMAN_STOP_WORDS[] = {
    L"einer", L"eine", L"eines", L"einem", L"einen", L"der", L"die",
    L"das", L"dass", L"da\x00df", L"du", L"er", L"sie", L"es", L"was",
    L"wer", L"wie", L"wir", L"und", L"oder", L"ohne", L"mit", L"am",
    L"im", L"in", L"aus", L"auf", L"ist", L"sein", L"war", L"wird",
    L"ihr", L"ihre", L"ihres", L"als", L"f\x00fcr", L"von", L"mit",
    L"dich", L"dir", L"mich", L"mir", L"mein", L"sein", L"kein",
    L"durch", L"wegen", L"wird"
};

GermanAnalyzer::GermanAnalyzer(LuceneVersion::Version matchVersion) {
    this->stopSet = getDefaultStopSet();
    this->matchVersion = matchVersion;
}

GermanAnalyzer::GermanAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords) {
    this->stopSet = stopwords;
    this->matchVersion = matchVersion;
}

GermanAnalyzer::GermanAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions) {
    this->stopSet = stopwords;
    this->exclusionSet = exclusions;
    this->matchVersion = matchVersion;
}

GermanAnalyzer::~GermanAnalyzer() {
}

const HashSet<String> GermanAnalyzer::getDefaultStopSet() {
    static HashSet<String> stopSet;
    if (!stopSet) {
        stopSet = HashSet<String>::newInstance(_GERMAN_STOP_WORDS, _GERMAN_STOP_WORDS + SIZEOF_ARRAY(_GERMAN_STOP_WORDS));
    }
    return stopSet;
}

void GermanAnalyzer::setStemExclusionTable(HashSet<String> exclusions) {
    exclusionSet = exclusions;
    setPreviousTokenStream(LuceneObjectPtr()); // force a new stemmer to be created
}

TokenStreamPtr GermanAnalyzer::tokenStream(const String& fieldName, const ReaderPtr& reader) {
    TokenStreamPtr result = newLucene<StandardTokenizer>(matchVersion, reader);
    result = newLucene<StandardFilter>(result);
    result = newLucene<LowerCaseFilter>(result);
    result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), result, stopSet);
    result = newLucene<GermanStemFilter>(result, exclusionSet);
    return result;
}

TokenStreamPtr GermanAnalyzer::reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
    GermanAnalyzerSavedStreamsPtr streams(boost::dynamic_pointer_cast<GermanAnalyzerSavedStreams>(getPreviousTokenStream()));
    if (!streams) {
        streams = newLucene<GermanAnalyzerSavedStreams>();
        streams->source = newLucene<StandardTokenizer>(matchVersion, reader);
        streams->result = newLucene<StandardFilter>(streams->source);
        streams->result = newLucene<LowerCaseFilter>(streams->result);
        streams->result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), streams->result, stopSet);
        streams->result = newLucene<GermanStemFilter>(streams->result, exclusionSet);
        setPreviousTokenStream(streams);
    } else {
        streams->source->reset(reader);
    }
    return streams->result;
}

GermanAnalyzerSavedStreams::~GermanAnalyzerSavedStreams() {
}

}
