/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "DutchAnalyzer.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "StopFilter.h"
#include "DutchStemFilter.h"

namespace Lucene {

const wchar_t* DutchAnalyzer::_DUTCH_STOP_WORDS[] = {
    L"de", L"en", L"van", L"ik", L"te", L"dat", L"die", L"in", L"een", L"hij", L"het", L"niet",
    L"zijn", L"is", L"was", L"op", L"aan", L"met", L"als", L"voor", L"had", L"er", L"maar",
    L"om", L"hem", L"dan", L"zou", L"of", L"wat", L"mijn", L"men", L"dit", L"zo", L"door",
    L"over", L"ze", L"zich", L"bij", L"ook", L"tot", L"je", L"mij", L"uit", L"der", L"daar",
    L"haar", L"naar", L"heb", L"hoe", L"heeft", L"hebben", L"deze", L"u", L"want", L"nog",
    L"zal", L"me", L"zij", L"nu", L"ge", L"geen", L"omdat", L"iets", L"worden", L"toch",
    L"al", L"waren", L"veel", L"meer", L"doen", L"toen", L"moet", L"ben", L"zonder", L"kan",
    L"hun", L"dus", L"alles", L"onder", L"ja", L"eens", L"hier", L"wie", L"werd", L"altijd",
    L"doch", L"wordt", L"wezen", L"kunnen", L"ons", L"zelf", L"tegen", L"na", L"reeds", L"wil",
    L"kon", L"niets", L"uw", L"iemand", L"geweest", L"andere"
};

DutchAnalyzer::DutchAnalyzer(LuceneVersion::Version matchVersion) {
    this->stoptable = getDefaultStopSet();
    this->excltable = HashSet<String>::newInstance();
    this->stemdict = MapStringString::newInstance();
    this->matchVersion = matchVersion;
}

DutchAnalyzer::DutchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords) {
    this->stoptable = stopwords;
    this->excltable = HashSet<String>::newInstance();
    this->matchVersion = matchVersion;
}

DutchAnalyzer::DutchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions) {
    this->stoptable = stopwords;
    this->excltable = exclusions;
    this->matchVersion = matchVersion;
}

DutchAnalyzer::~DutchAnalyzer() {
}

void DutchAnalyzer::initialize() {
    stemdict.put(L"fiets", L"fiets"); // otherwise fiet
    stemdict.put(L"bromfiets", L"bromfiets"); // otherwise bromfiet
    stemdict.put(L"ei", L"eier");
    stemdict.put(L"kind", L"kinder");
}

const HashSet<String> DutchAnalyzer::getDefaultStopSet() {
    static HashSet<String> stoptable;
    if (!stoptable) {
        stoptable = HashSet<String>::newInstance(_DUTCH_STOP_WORDS, _DUTCH_STOP_WORDS + SIZEOF_ARRAY(_DUTCH_STOP_WORDS));
    }
    return stoptable;
}

void DutchAnalyzer::setStemExclusionTable(HashSet<String> exclusions) {
    excltable = exclusions;
    setPreviousTokenStream(LuceneObjectPtr()); // force a new stemmer to be created
}

TokenStreamPtr DutchAnalyzer::tokenStream(const String& fieldName, const ReaderPtr& reader) {
    TokenStreamPtr result = newLucene<StandardTokenizer>(matchVersion, reader);
    result = newLucene<StandardFilter>(result);
    result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), result, stoptable);
    result = newLucene<DutchStemFilter>(result, excltable);
    return result;
}

TokenStreamPtr DutchAnalyzer::reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
    DutchAnalyzerSavedStreamsPtr streams(boost::dynamic_pointer_cast<DutchAnalyzerSavedStreams>(getPreviousTokenStream()));
    if (!streams) {
        streams = newLucene<DutchAnalyzerSavedStreams>();
        streams->source = newLucene<StandardTokenizer>(matchVersion, reader);
        streams->result = newLucene<StandardFilter>(streams->source);
        streams->result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), streams->result, stoptable);
        streams->result = newLucene<DutchStemFilter>(streams->result, excltable);
        setPreviousTokenStream(streams);
    } else {
        streams->source->reset(reader);
    }
    return streams->result;
}

DutchAnalyzerSavedStreams::~DutchAnalyzerSavedStreams() {
}

}
