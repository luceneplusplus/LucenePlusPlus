/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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
#include "EnglishPossessiveFilter.h"
#include "TurkishLowerCaseFilter.h"

namespace Lucene
{
    SnowballAnalyzer::SnowballAnalyzer(LuceneVersion::Version matchVersion, const String& name)
    {
        this->matchVersion = matchVersion;
        this->name = name;
    }

    SnowballAnalyzer::SnowballAnalyzer(LuceneVersion::Version matchVersion, const String& name, HashSet<String> stopwords)
    {
        this->stopSet = stopwords;
        this->matchVersion = matchVersion;
        this->name = name;
    }

    SnowballAnalyzer::~SnowballAnalyzer()
    {
    }

    TokenStreamPtr SnowballAnalyzer::tokenStream(const String& fieldName, ReaderPtr reader)
    {
        TokenStreamPtr result(newLucene<StandardTokenizer>(matchVersion, reader));
        result = newLucene<StandardFilter>(matchVersion, result);
        // remove the possessive 's for english stemmers
        if (LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31) &&
            (name == L"English" || name == L"Porter" || name == L"Lovins"))
            result = newLucene<EnglishPossessiveFilter>(result);
        // Use a special lowercase filter for turkish, the stemmer expects it.
        if (LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31) && name == L"Turkish")
            result = newLucene<TurkishLowerCaseFilter>(result);
        else
            result = newLucene<LowerCaseFilter>(matchVersion, result);
        if (stopSet)
            result = newLucene<StopFilter>(matchVersion, result, stopSet);
        result = newLucene<SnowballFilter>(result, name);
        return result;
    }

    TokenStreamPtr SnowballAnalyzer::reusableTokenStream(const String& fieldName, ReaderPtr reader)
    {
        SnowballAnalyzerSavedStreamsPtr streams(boost::dynamic_pointer_cast<SnowballAnalyzerSavedStreams>(getPreviousTokenStream()));
        if (!streams)
        {
            streams = newLucene<SnowballAnalyzerSavedStreams>();
            streams->source = newLucene<StandardTokenizer>(matchVersion, reader);
            streams->result = newLucene<StandardFilter>(matchVersion, streams->source);
            // Use a special lowercase filter for turkish, the stemmer expects it.
            if (LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31) && name == L"Turkish")
                streams->result = newLucene<TurkishLowerCaseFilter>(streams->result);
            else
                streams->result = newLucene<LowerCaseFilter>(matchVersion, streams->result);
            if (stopSet)
                streams->result = newLucene<StopFilter>(matchVersion, streams->result, stopSet);
            streams->result = newLucene<SnowballFilter>(streams->result, name);
            setPreviousTokenStream(streams);
        }
        else
            streams->source->reset(reader);
        return streams->result;
    }

    SnowballAnalyzerSavedStreams::~SnowballAnalyzerSavedStreams()
    {
    }
}

