/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "Directory.h"
#include "LockFactory.h"
#include "BufferedIndexOutput.h"
#include "IndexFileNameFilter.h"
#include "IndexInput.h"
#include "IndexOutput.h"

namespace Lucene
{
    Directory::Directory()
    {
        isOpen = true;
    }
    
    Directory::~Directory()
    {
    }
    
    void Directory::close()
    {
        // override
    }
    
    void Directory::sync(const String& name)
    {
    }
    
    void Directory::sync(HashSet<String> names)
    {
        for (HashSet<String>::iterator name = names.begin(); name != names.end(); ++name)
            sync(*name);
    }
    
    IndexInputPtr Directory::openInput(const String& name, int32_t bufferSize)
    {
        return openInput(name);
    }
    
    LockPtr Directory::makeLock(const String& name)
    {
        return lockFactory->makeLock(name);
    }
    
    void Directory::clearLock(const String& name)
    {
        if (lockFactory)
            lockFactory->clearLock(name);
    }
    
    void Directory::setLockFactory(LockFactoryPtr lockFactory)
    {
        BOOST_ASSERT(lockFactory);
        this->lockFactory = lockFactory;
        this->lockFactory->setLockPrefix(getLockID());
    }
    
    LockFactoryPtr Directory::getLockFactory()
    {
        return lockFactory;
    }
    
    String Directory::getLockID()
    {
        return toString();
    }
    
    String Directory::toString()
    {
        return LuceneObject::toString() + L" lockFactory=" + getLockFactory()->toString();
    }
    
    void Directory::copy(DirectoryPtr to, const String& src, const String& dest)
    {
        IndexOutputPtr os(to->createOutput(dest));
        IndexInputPtr is(openInput(src));
        LuceneException finally;
        try
        {
            is->copyBytes(os, is->length());
        }
        catch (IOException&)
        {
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        os->close();
        is->close();
        finally.throwException();
    }
    
    void Directory::copy(DirectoryPtr src, DirectoryPtr dest, bool closeDirSrc)
    {
        IndexFileNameFilterPtr filter(IndexFileNameFilter::getFilter());
        HashSet<String> files(src->listAll());
        for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file)
        {
            if (filter->accept(L"", *file))
                src->copy(dest, *file, *file);
        }
        if (closeDirSrc)
            src->close();
    }
    
    void Directory::ensureOpen()
    {
        if (!isOpen)
            boost::throw_exception(AlreadyClosedException(L"This directory is closed"));
    }
}
