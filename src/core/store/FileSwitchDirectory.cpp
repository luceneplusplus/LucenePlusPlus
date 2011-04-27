/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "FileSwitchDirectory.h"

namespace Lucene
{
    FileSwitchDirectory::FileSwitchDirectory(HashSet<String> primaryExtensions, DirectoryPtr primaryDir, DirectoryPtr secondaryDir, bool doClose)
    {
        this->primaryExtensions = primaryExtensions;
        this->primaryDir = primaryDir;
        this->secondaryDir = secondaryDir;
        this->doClose = doClose;
        this->lockFactory = primaryDir->getLockFactory();
    }
    
    FileSwitchDirectory::~FileSwitchDirectory()
    {
    }
    
    DirectoryPtr FileSwitchDirectory::getPrimaryDir()
    {
        return primaryDir;
    }
    
    DirectoryPtr FileSwitchDirectory::getSecondaryDir()
    {
        return secondaryDir;
    }
    
    void FileSwitchDirectory::close()
    {
        if (doClose)
        {
            LuceneException finally;
            try
            {
                secondaryDir->close();
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            doClose = false;
            primaryDir->close();
            finally.throwException();
        }
    }
    
    HashSet<String> FileSwitchDirectory::listAll()
    {
        HashSet<String> primaryFiles(primaryDir->listAll());
        HashSet<String> secondaryFiles(secondaryDir->listAll());
        HashSet<String> files(HashSet<String>::newInstance(primaryFiles.begin(), primaryFiles.end()));
        files.addAll(secondaryFiles.begin(), secondaryFiles.end());
        return files;
    }
    
    String FileSwitchDirectory::getExtension(const String& name)
    {
        String::size_type i = name.find_last_of(L'.');
        return i == String::npos ? L"" : name.substr(i + 1);
    }
    
    DirectoryPtr FileSwitchDirectory::getDirectory(const String& name)
    {
        return primaryExtensions.contains(getExtension(name)) ? primaryDir : secondaryDir;
    }
    
    bool FileSwitchDirectory::fileExists(const String& name)
    {
        return getDirectory(name)->fileExists(name);
    }
    
    uint64_t FileSwitchDirectory::fileModified(const String& name)
    {
        return getDirectory(name)->fileModified(name);
    }
    
    void FileSwitchDirectory::touchFile(const String& name)
    {
        getDirectory(name)->touchFile(name);
    }
    
    void FileSwitchDirectory::deleteFile(const String& name)
    {
        getDirectory(name)->deleteFile(name);
    }
    
    int64_t FileSwitchDirectory::fileLength(const String& name)
    {
        return getDirectory(name)->fileLength(name);
    }
    
    IndexOutputPtr FileSwitchDirectory::createOutput(const String& name)
    {
        return getDirectory(name)->createOutput(name);
    }
    
    void FileSwitchDirectory::sync(const String& name)
    {
        HashSet<String> names(HashSet<String>::newInstance());
        names.add(name);
        sync(names);
    }
    
    void FileSwitchDirectory::sync(HashSet<String> names)
    {
        HashSet<String> primaryNames(HashSet<String>::newInstance());
        HashSet<String> secondaryNames(HashSet<String>::newInstance());

        for (HashSet<String>::iterator name = names.begin(); name != names.end(); ++name)
        {
            if (primaryExtensions.contains(getExtension(*name)))
                primaryNames.add(*name);
            else
                secondaryNames.add(*name);
        }

        primaryDir->sync(primaryNames);
        secondaryDir->sync(secondaryNames);
    }
    
    IndexInputPtr FileSwitchDirectory::openInput(const String& name)
    {
        return getDirectory(name)->openInput(name);
    }
}
