/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Reader.h"

namespace Lucene
{
	/// Read text from a character-input stream, buffering characters so as to provide 
	/// for the efficient reading of characters, arrays, and lines.
	class LPPAPI BufferedReader : public Reader
	{
	public:
		/// Create a buffering character-input stream.
		BufferedReader(ReaderPtr reader);
		virtual ~BufferedReader();
		
		LUCENE_CLASS(BufferedReader);
	
	protected:
		ReaderPtr reader;
		int32_t bufferStart; // position in file of buffer
		int32_t bufferLength; // end of valid bytes
		int32_t bufferPosition; // next byte to read
		CharArray buffer;
	
	public:
		static const int32_t READER_BUFFER;
	
	public:
		using Reader::read;
		
		/// Read a single character.
		virtual int32_t read();
		
		/// Read characters into a portion of an array.
		virtual int32_t read(wchar_t* b, int32_t offset, int32_t length);
		
		/// Read a line of text.
		virtual String readLine();

		/// Close the stream.
		virtual void close();
		
		/// Tell whether this stream supports the mark() operation
		virtual bool markSupported();
		
		/// Reset the stream.
		virtual void reset();
	
	protected:
		/// Refill buffer in preparation for reading.
		void refill();
		
		/// Read a single character without moving position.
		int32_t peek();
	};
}
