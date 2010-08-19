/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FSDirectory.h"
#include "BufferedIndexInput.h"
#include "BufferedIndexOutput.h"

namespace Lucene
{
	/// A straightforward implementation of {@link FSDirectory} using std::ofstream and std::ifstream.
	class LPPAPI SimpleFSDirectory : public FSDirectory
	{
	public:
		/// Create a new SimpleFSDirectory for the named location  and {@link NativeFSLockFactory}.
		/// @param path the path of the directory.
		/// @param lockFactory the lock factory to use, or null for the default ({@link NativeFSLockFactory})
		SimpleFSDirectory(const String& path, LockFactoryPtr lockFactory = LockFactoryPtr());
		virtual ~SimpleFSDirectory();
		
		LUCENE_CLASS(SimpleFSDirectory);
				
	public:
		/// Creates an IndexOutput for the file with the given name.
		virtual IndexOutputPtr createOutput(const String& name);
		
		/// Returns a stream reading an existing file, with the specified read buffer size.  The particular Directory implementation may ignore the buffer size.
		virtual IndexInputPtr openInput(const String& name);
		
		/// Creates an IndexInput for the file with the given name.
		virtual IndexInputPtr openInput(const String& name, int32_t bufferSize);
	};
	
	class LPPAPI InputFile : public LuceneObject
	{
	public:
		InputFile(const String& path);
		virtual ~InputFile();
		
		LUCENE_CLASS(InputFile);
				
	public:
		static const int32_t FILE_EOF;
		static const int32_t FILE_ERROR;
	
	protected:
		ifstreamPtr file;
		int64_t position;
		int64_t length;
	
	public:
		void setPosition(int64_t position);
		int64_t getPosition();
		int64_t getLength();
		int32_t read(uint8_t* b, int32_t offset, int32_t length);
		void close();
		bool isValid();
	};
	
	class LPPAPI SimpleFSIndexInput : public BufferedIndexInput
	{
	public:
		SimpleFSIndexInput();
		SimpleFSIndexInput(const String& path, int32_t bufferSize, int32_t chunkSize);
		virtual ~SimpleFSIndexInput();
		
		LUCENE_CLASS(SimpleFSIndexInput);
				
	protected:
		String path;
		InputFilePtr file;
		bool isClone;
		int32_t chunkSize;
	
	protected:
		virtual void readInternal(uint8_t* b, int32_t offset, int32_t length);
		virtual void seekInternal(int64_t pos);
		
	public:
		virtual int64_t length();
		virtual void close();
		
		/// Method used for testing.
		bool isValid();
		
		/// Returns a clone of this stream.
		virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
	};
	
	class LPPAPI OutputFile : public LuceneObject
	{
	public:
		OutputFile(const String& path);
		virtual ~OutputFile();
		
		LUCENE_CLASS(OutputFile);
				
	protected:
		ofstreamPtr file;
		String path;
	
	public:
		bool write(const uint8_t* b, int32_t offset, int32_t length);
		void close();
		void setPosition(int64_t position);
		int64_t getLength();
		void setLength(int64_t length);
		void flush();
		bool isValid();
	};
		
	class LPPAPI SimpleFSIndexOutput : public BufferedIndexOutput
	{
	public:
		SimpleFSIndexOutput(const String& path);
		virtual ~SimpleFSIndexOutput();
		
		LUCENE_CLASS(SimpleFSIndexOutput);
				
	protected:
		OutputFilePtr file;
		bool isOpen;
	
	public:
		virtual void flushBuffer(const uint8_t* b, int32_t offset, int32_t length);
		virtual void close();
		virtual void seek(int64_t pos);
		virtual int64_t length();
		virtual void setLength(int64_t length);
	};		
}
