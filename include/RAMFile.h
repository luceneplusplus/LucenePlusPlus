/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	/// File used as buffer in RAMDirectory
	class LPPAPI RAMFile : public LuceneObject
	{
	public:
		RAMFile(); // File used as buffer, in no RAMDirectory
		RAMFile(RAMDirectoryPtr directory);
		virtual ~RAMFile();
		
		LUCENE_CLASS(RAMFile);
				
	public:
		int64_t length;
		RAMDirectoryWeakPtr _directory;
		int64_t sizeInBytes; // Only maintained if in a directory; updates synchronized on directory
	
	protected:
		Collection<ByteArray> buffers;
		
		/// This is publicly modifiable via Directory.touchFile(), so direct access not supported
		int64_t lastModified;
		
	public:
		/// For non-stream access from thread that might be concurrent with writing.
		int64_t getLength();		
		void setLength(int64_t length);
		
		/// For non-stream access from thread that might be concurrent with writing
		int64_t getLastModified();
		void setLastModified(int64_t lastModified);
		
		ByteArray addBuffer(int32_t size);
		ByteArray getBuffer(int32_t index);
		int32_t numBuffers();
		
		/// Only valid if in a directory
		int64_t getSizeInBytes();
	
	protected:
		/// Allocate a new buffer.  Subclasses can allocate differently. 
		virtual ByteArray newBuffer(int32_t size);
	};
}
