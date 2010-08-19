/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lucene.h"

namespace Lucene
{
	/// Utility class to support streaming info messages.
	class LPPAPI InfoStream : public LuceneObject
	{
	public:
		virtual ~InfoStream();
		LUCENE_CLASS(InfoStream);
			
	public:
		virtual InfoStream& operator<< (const String& t) = 0;
	};
	
	/// Stream override to write messages to a file.
	class LPPAPI InfoStreamFile : public InfoStream
	{
	public:
		InfoStreamFile(const String& path);
		virtual ~InfoStreamFile();
		
		LUCENE_CLASS(InfoStreamFile);
			
	protected:
		std::wofstream file;
	
	public:
		virtual InfoStreamFile& operator<< (const String& t);
	};
	
	/// Stream override to write messages to a std::cout.
	class LPPAPI InfoStreamOut : public InfoStream
	{
	public:
		virtual ~InfoStreamOut();
		LUCENE_CLASS(InfoStreamOut);
			
	public:
		virtual InfoStreamOut& operator<< (const String& t);
	};
}
