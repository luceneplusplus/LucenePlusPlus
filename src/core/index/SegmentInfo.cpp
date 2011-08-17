/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include <boost/algorithm/string.hpp>
#include "SegmentInfo.h"
#include "SegmentInfos.h"
#include "Directory.h"
#include "IndexInput.h"
#include "IndexOutput.h"
#include "IndexFileNames.h"
#include "IndexFileNameFilter.h"
#include "BitVector.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"
#include "StringUtils.h"
#include "CompoundFileReader.h"

namespace Lucene
{
    const int32_t SegmentInfo::NO = -1; // no norms; no deletes;
    const int32_t SegmentInfo::YES = 1; // have norms; have deletes;
    const int32_t SegmentInfo::CHECK_DIR = 0; // must check dir to see if there are norms/deletions
    const int32_t SegmentInfo::WITHOUT_GEN = 0; // a file name that has no GEN in it. 
    
    SegmentInfo::SegmentInfo(const String& name, int32_t docCount, DirectoryPtr dir, bool isCompoundFile, bool hasSingleNormFile, 
                             bool hasProx, bool hasVectors)
    {
        sizeInBytesNoStore = -1;
        sizeInBytesWithStore = -1;
        docStoreIsCompoundFile = false;
        
        this->name = name;
        this->docCount = docCount;
        this->dir = dir;
        delGen = NO;
        this->isCompoundFile = (uint8_t)(isCompoundFile ? YES : NO);
        preLockless = false;
        this->hasSingleNormFile = hasSingleNormFile;
        this->docStoreOffset = -1;
        delCount = 0;
        this->hasProx = hasProx;
        this->hasVectors = hasVectors;
        this->version = Constants::LUCENE_MAIN_VERSION;
    }
            
    SegmentInfo::SegmentInfo(DirectoryPtr dir, int32_t format, IndexInputPtr input)
    {
        sizeInBytesNoStore = -1;
        sizeInBytesWithStore = -1;
        this->dir = dir;
        if (format <= SegmentInfos::FORMAT_3_1)
            version = input->readString();
        name = input->readString();
        docCount = input->readInt();
        if (format <= SegmentInfos::FORMAT_LOCKLESS)
        {
            delGen = input->readLong();
            if (format <= SegmentInfos::FORMAT_SHARED_DOC_STORE)
            {
                docStoreOffset = input->readInt();
                if (docStoreOffset != -1)
                {
                    docStoreSegment = input->readString();
                    docStoreIsCompoundFile = (input->readByte() == 1);
                }
                else
                {
                    docStoreSegment = name;
                    docStoreIsCompoundFile = false;
                }
            }
            else
            {
                docStoreOffset = -1;
                docStoreSegment = name;
                docStoreIsCompoundFile = false;
            }
            if (format <= SegmentInfos::FORMAT_SINGLE_NORM_FILE)
                hasSingleNormFile = (input->readByte() == 1);
            else
                hasSingleNormFile = false;
            int32_t numNormGen = input->readInt();
            if (numNormGen != NO)
            {
                normGen = Collection<int64_t>::newInstance(numNormGen);
                for (int32_t j = 0; j < numNormGen; ++j)
                    normGen[j] = input->readLong();
            }
            isCompoundFile = input->readByte();
            preLockless = (isCompoundFile == CHECK_DIR);
            if (format <= SegmentInfos::FORMAT_DEL_COUNT)
            {
                delCount = input->readInt();
                BOOST_ASSERT(delCount <= docCount);
            }
            else
                delCount = -1;
            if (format <= SegmentInfos::FORMAT_HAS_PROX)
                hasProx = (input->readByte() == 1);
            else
                hasProx = true;
            
            if (format <= SegmentInfos::FORMAT_DIAGNOSTICS)
                diagnostics = input->readStringStringMap();
            else
                diagnostics = MapStringString::newInstance();
            
            if (format <= SegmentInfos::FORMAT_HAS_VECTORS)
                hasVectors = (input->readByte() == 1);
            else
            {
                String storesSegment;
                String ext;
                bool isCompoundFile = false;
                if (docStoreOffset != -1)
                {
                    storesSegment = docStoreSegment;
                    isCompoundFile = docStoreIsCompoundFile;
                    ext = IndexFileNames::COMPOUND_FILE_STORE_EXTENSION();
                }
                else
                {
                    storesSegment = name;
                    isCompoundFile = getUseCompoundFile();
                    ext = IndexFileNames::COMPOUND_FILE_EXTENSION();
                }
                DirectoryPtr dirToTest;
                if (isCompoundFile)
                    dirToTest = newLucene<CompoundFileReader>(dir, IndexFileNames::segmentFileName(storesSegment, ext));
                else
                    dirToTest = dir;
                LuceneException finally;
                try
                {
                    hasVectors = dirToTest->fileExists(IndexFileNames::segmentFileName(storesSegment, IndexFileNames::VECTORS_INDEX_EXTENSION()));
                }
                catch (LuceneException& e)
                {
                    finally = e;
                }
                if (isCompoundFile)
                    dirToTest->close();
                finally.throwException();
            }
        }
        else
        {
            delGen = CHECK_DIR;
            isCompoundFile = CHECK_DIR;
            preLockless = true;
            hasSingleNormFile = false;
            docStoreOffset = -1;
            docStoreIsCompoundFile = false;
            delCount = -1;
            hasProx = true;
            hasVectors = false;
            diagnostics = MapStringString::newInstance();
        }
    }
    
    SegmentInfo::~SegmentInfo()
    {
    }
    
    void SegmentInfo::reset(SegmentInfoPtr src)
    {
        clearFiles();
        name = src->name;
        docCount = src->docCount;
        dir = src->dir;
        preLockless = src->preLockless;
        delGen = src->delGen;
        docStoreOffset = src->docStoreOffset;
        docStoreIsCompoundFile = src->docStoreIsCompoundFile;
        if (!src->normGen)
            normGen = src->normGen;
        else
            normGen = Collection<int64_t>::newInstance(src->normGen.begin(), src->normGen.end());
        isCompoundFile = src->isCompoundFile;
        hasSingleNormFile = src->hasSingleNormFile;
        delCount = src->delCount;
    }
    
    void SegmentInfo::setDiagnostics(MapStringString diagnostics)
    {
        this->diagnostics = diagnostics;
    }
    
    MapStringString SegmentInfo::getDiagnostics()
    {
        return diagnostics;
    }
    
    void SegmentInfo::setNumFields(int32_t numFields)
    {
        if (!normGen)
        {
            // normGen is null if we loaded a pre-2.1 segment file, or, if this segments file hasn't had any
            // norms set against it yet
            normGen = Collection<int64_t>::newInstance(numFields);
            
            if (!preLockless) 
            { // Do nothing: thus leaving normGen[k] == CHECK_DIR (==0), so that later we know  
            } // we have to check filesystem for norm files, because this is prelockless.
            else
            {
                // This is a FORMAT_LOCKLESS segment, which means there are no separate norms
                for (int32_t i = 0; i < numFields; ++i)
                    normGen[i] = NO;
            }
        }
    }
    
    int64_t SegmentInfo::sizeInBytes(bool includeDocStores)
    {
        if (includeDocStores)
        {
            if (sizeInBytesWithStore != -1)
                return sizeInBytesWithStore;
            int64_t sum = 0;
            HashSet<String> _files(files());
            for (HashSet<String>::iterator fileName = _files.begin(); fileName != _files.end(); ++fileName)
            {
                // We don't count bytes used by a shared doc store against this segment
                if (docStoreOffset == -1 || !IndexFileNames::isDocStoreFile(*fileName))
                    sum += dir->fileLength(*fileName);
            }
            sizeInBytesWithStore = sum;
            return sizeInBytesWithStore;
        }
        else
        {
            if (sizeInBytesNoStore != -1)
                return sizeInBytesNoStore;
            int64_t sum = 0;
            HashSet<String> _files(files());
            for (HashSet<String>::iterator fileName = _files.begin(); fileName != _files.end(); ++fileName)
            {
                if (IndexFileNames::isDocStoreFile(*fileName))
                    continue;
                sum += dir->fileLength(*fileName);
            }
            sizeInBytesNoStore = sum;
            return sizeInBytesNoStore;
        }
    }
    
    bool SegmentInfo::getHasVectors()
    {
        return hasVectors;
    }
    
    void SegmentInfo::setHasVectors(bool v)
    {
        hasVectors = v;
        clearFiles();
    }
    
    bool SegmentInfo::hasDeletions()
    {
        if (delGen == NO)
            return false;
        else if (delGen >= YES)
            return true;
        else
            return dir->fileExists(getDelFileName());
    }
    
    void SegmentInfo::advanceDelGen()
    {
        // delGen 0 is reserved for pre-LOCKLESS format
        if (delGen == NO)
            delGen = YES;
        else
            delGen++;
        clearFiles();
    }
    
    void SegmentInfo::clearDelGen()
    {
        delGen = NO;
        clearFiles();
    }
    
    LuceneObjectPtr SegmentInfo::clone(LuceneObjectPtr other)
    {
        SegmentInfoPtr si = newLucene<SegmentInfo>(name, docCount, dir, false, hasSingleNormFile, hasProx, hasVectors);
        si->docStoreOffset = docStoreOffset;
        si->docStoreSegment = docStoreSegment;
        si->docStoreIsCompoundFile = docStoreIsCompoundFile;
        si->delGen = delGen;
        si->delCount = delCount;
        si->preLockless = preLockless;
        si->isCompoundFile = isCompoundFile;
        si->diagnostics = MapStringString::newInstance();
        si->diagnostics.putAll(diagnostics.begin(), diagnostics.end());
        if (normGen)
            si->normGen = Collection<int64_t>::newInstance(normGen.begin(), normGen.end());
        si->version = version;
        return si;
    }
    
    String SegmentInfo::getDelFileName()
    {
        if (delGen == NO)
        {
            // in this case we know there is no deletion filename against this segment
            return L"";
        }
        else
        {
            // if delgen is check_dir, it's the pre-lockless-commit file format
            return IndexFileNames::fileNameFromGeneration(name, IndexFileNames::DELETES_EXTENSION(), delGen); 
        }
    }
    
    bool SegmentInfo::hasSeparateNorms(int32_t fieldNumber)
    {
        if ((!normGen && preLockless) || (normGen && normGen[fieldNumber] == CHECK_DIR))
        {
            // must fallback to directory file exists check
            return dir->fileExists(name + L".s" + StringUtils::toString(fieldNumber));
        }
        else if (!normGen || normGen[fieldNumber] == NO)
            return false;
        else
            return true;
    }
    
    bool SegmentInfo::hasSeparateNorms()
    {
        if (!normGen)
        {
            if (!preLockless)
            {
                // this means we were created with lockless code and no norms are written yet
                return false;
            }
            else
            {
                HashSet<String> result(dir->listAll());
                if (!result)
                    boost::throw_exception(IOException(L"Cannot read directory " + dir->toString() + L": listAll() returned null"));
                String pattern(name + L".s");
                int32_t patternLength = pattern.length();
                for (HashSet<String>::iterator fileName = result.begin(); fileName != result.end(); ++fileName)
                {
                    if (IndexFileNameFilter::accept(L"", *fileName) && boost::starts_with(*fileName, pattern) && UnicodeUtil::isDigit((*fileName)[patternLength]))
                        return true;
                }
                return false;
            }
        }
        else
        {
            // This means this segment was saved with LOCKLESS code so we first check whether any normGen's are >= 1
            // (meaning they definitely have separate norms)
            for (Collection<int64_t>::iterator gen = normGen.begin(); gen != normGen.end(); ++gen)
            {
                if (*gen >= YES)
                    return true;
            }

            // Next we look for any == 0.  These cases were pre-LOCKLESS and must be checked in directory
            for (int32_t gen = 0; gen < normGen.size(); ++gen)
            {
                if (normGen[gen] == CHECK_DIR && hasSeparateNorms(gen))
                    return true;
            }
        }
        return false;
    }
    
    void SegmentInfo::advanceNormGen(int32_t fieldIndex)
    {
        if (normGen[fieldIndex] == NO)
            normGen[fieldIndex] = YES;
        else
            normGen[fieldIndex]++;
        clearFiles();
    }
    
    String SegmentInfo::getNormFileName(int32_t number)
    {
        int64_t gen = !normGen ? CHECK_DIR : normGen[number];
        
        if (hasSeparateNorms(number))
        {
            // case 1: separate norm
            return IndexFileNames::fileNameFromGeneration(name, L"s" + StringUtils::toString(number), gen);
        }
        
        if (hasSingleNormFile)
        {
            // case 2: lockless (or nrm file exists) - single file for all norms 
            return IndexFileNames::fileNameFromGeneration(name, IndexFileNames::NORMS_EXTENSION(), WITHOUT_GEN);
        }
        
        // case 3: norm file for each field
        return IndexFileNames::fileNameFromGeneration(name, L"f" + StringUtils::toString(number), WITHOUT_GEN);
    }
    
    void SegmentInfo::setUseCompoundFile(bool isCompoundFile)
    {
        this->isCompoundFile = (uint8_t)(isCompoundFile ? YES : NO);
        clearFiles();
    }
    
    bool SegmentInfo::getUseCompoundFile()
    {
        if (isCompoundFile == (uint8_t)NO)
            return false;
        else if (isCompoundFile == (uint8_t)YES)
            return true;
        else
            return dir->fileExists(IndexFileNames::segmentFileName(name, IndexFileNames::COMPOUND_FILE_EXTENSION()));
    }
    
    int32_t SegmentInfo::getDelCount()
    {
        if (delCount == -1)
            delCount = hasDeletions() ? BitVector(dir, getDelFileName()).count() : 0;
        BOOST_ASSERT(delCount <= docCount);
        return delCount;
    }
    
    void SegmentInfo::setDelCount(int32_t delCount)
    {
        this->delCount = delCount;
        BOOST_ASSERT(delCount <= docCount);
    }
    
    int32_t SegmentInfo::getDocStoreOffset()
    {
        return docStoreOffset;
    }
    
    bool SegmentInfo::getDocStoreIsCompoundFile()
    {
        return docStoreIsCompoundFile;
    }
    
    void SegmentInfo::setDocStoreIsCompoundFile(bool v)
    {
        docStoreIsCompoundFile = v;
        clearFiles();
    }
    
    String SegmentInfo::getDocStoreSegment()
    {
        return docStoreSegment;
    }
    
    void SegmentInfo::setDocStoreSegment(const String& segment)
    {
        docStoreSegment = segment;
    }
    
    void SegmentInfo::setDocStoreOffset(int32_t offset)
    {
        docStoreOffset = offset;
        clearFiles();
    }
    
    void SegmentInfo::setDocStore(int32_t offset, const String& segment, bool isCompoundFile)
    {
        docStoreOffset = offset;
        docStoreSegment = segment;
        docStoreIsCompoundFile = isCompoundFile;
        clearFiles();
    }
    
    void SegmentInfo::write(IndexOutputPtr output)
    {
        BOOST_ASSERT(delCount <= docCount);
        // Write the Lucene version that created this segment, since 3.1
        output->writeString(version); 
        output->writeString(name);
        output->writeInt(docCount);
        output->writeLong(delGen);
        output->writeInt(docStoreOffset);
        if (docStoreOffset != -1)
        {
            output->writeString(docStoreSegment);
            output->writeByte((uint8_t)(docStoreIsCompoundFile ? 1 : 0));
        }
        
        output->writeByte((uint8_t)(hasSingleNormFile ? 1 : 0));
        if (!normGen)
            output->writeInt(NO);
        else
        {
            output->writeInt(normGen.size());
            for (Collection<int64_t>::iterator gen = normGen.begin(); gen != normGen.end(); ++gen)
                output->writeLong(*gen);
        }
        output->writeByte(isCompoundFile);
        output->writeInt(delCount);
        output->writeByte((uint8_t)(hasProx ? 1 : 0));
        output->writeStringStringMap(diagnostics);
        output->writeByte((uint8_t)(hasVectors ? 1 : 0));
    }
    
    void SegmentInfo::setHasProx(bool hasProx)
    {
        this->hasProx = hasProx;
        clearFiles();
    }
    
    bool SegmentInfo::getHasProx()
    {
        return hasProx;
    }
    
    void SegmentInfo::addIfExists(HashSet<String> files, const String& fileName)
    {
        if (dir->fileExists(fileName))
            files.add(fileName);
    }
    
    HashSet<String> SegmentInfo::files()
    {
        if (_files)
        {
            // already cached
            return _files;
        }
        
        _files = HashSet<String>::newInstance();
        bool useCompoundFile = getUseCompoundFile();
        
        if (useCompoundFile)
            _files.add(IndexFileNames::segmentFileName(name, IndexFileNames::COMPOUND_FILE_EXTENSION()));
        else
        {
            for (HashSet<String>::iterator ext = IndexFileNames::NON_STORE_INDEX_EXTENSIONS().begin(); ext != IndexFileNames::NON_STORE_INDEX_EXTENSIONS().end(); ++ext)
                addIfExists(_files, IndexFileNames::segmentFileName(name, *ext));
        }
        
        if (docStoreOffset != -1)
        {
            // we are sharing doc stores (stored fields, term vectors) with other segments
            BOOST_ASSERT(!docStoreSegment.empty());
            if (docStoreIsCompoundFile)
                _files.add(IndexFileNames::segmentFileName(docStoreSegment, IndexFileNames::COMPOUND_FILE_STORE_EXTENSION()));
            else
            {
                _files.add(IndexFileNames::segmentFileName(docStoreSegment, IndexFileNames::FIELDS_INDEX_EXTENSION()));
                _files.add(IndexFileNames::segmentFileName(docStoreSegment, IndexFileNames::FIELDS_EXTENSION()));
                if (hasVectors)
                {
                    _files.add(IndexFileNames::segmentFileName(docStoreSegment, IndexFileNames::VECTORS_INDEX_EXTENSION()));
                    _files.add(IndexFileNames::segmentFileName(docStoreSegment, IndexFileNames::VECTORS_DOCUMENTS_EXTENSION()));
                    _files.add(IndexFileNames::segmentFileName(docStoreSegment, IndexFileNames::VECTORS_FIELDS_EXTENSION()));
                }
            }
        }
        else if (!useCompoundFile)
        {
            _files.add(IndexFileNames::segmentFileName(name, IndexFileNames::FIELDS_INDEX_EXTENSION()));
            _files.add(IndexFileNames::segmentFileName(name, IndexFileNames::FIELDS_EXTENSION()));
            if (hasVectors)
            {
                _files.add(IndexFileNames::segmentFileName(name, IndexFileNames::VECTORS_INDEX_EXTENSION()));
                _files.add(IndexFileNames::segmentFileName(name, IndexFileNames::VECTORS_DOCUMENTS_EXTENSION()));
                _files.add(IndexFileNames::segmentFileName(name, IndexFileNames::VECTORS_FIELDS_EXTENSION()));
            }
        }
        
        String delFileName(IndexFileNames::fileNameFromGeneration(name, IndexFileNames::DELETES_EXTENSION(), delGen));
        if (!delFileName.empty() && (delGen >= YES || dir->fileExists(delFileName)))
            _files.add(delFileName);
        
        // careful logic for norms files
        if (normGen)
        {
            for (int32_t i = 0; i < normGen.size(); ++i)
            {
                int64_t gen = normGen[i];
                if (gen >= YES)
                {
                    // definitely a separate norm file, with generation
                    _files.add(IndexFileNames::fileNameFromGeneration(name, IndexFileNames::SEPARATE_NORMS_EXTENSION() + StringUtils::toString(i), gen));
                }
                else if (gen == NO)
                {
                    // no separate norms but maybe plain norms in the non compound file case
                    if (!hasSingleNormFile && !useCompoundFile)
                    {
                        String fileName(IndexFileNames::segmentFileName(name, IndexFileNames::PLAIN_NORMS_EXTENSION() + StringUtils::toString(i)));
                        if (dir->fileExists(fileName))
                            _files.add(fileName);
                    }
                }
                else if (gen == CHECK_DIR)
                {
                    // pre-2.1: we have to check file existence
                    String fileName;
                    if (useCompoundFile)
                        fileName = IndexFileNames::segmentFileName(name, IndexFileNames::SEPARATE_NORMS_EXTENSION() + StringUtils::toString(i));
                    else if (!hasSingleNormFile)
                        fileName = IndexFileNames::segmentFileName(name, IndexFileNames::PLAIN_NORMS_EXTENSION() + StringUtils::toString(i));
                    if (!fileName.empty() && dir->fileExists(fileName))
                        _files.add(fileName);
                }
            }
        }
        else if (preLockless || (!hasSingleNormFile && !useCompoundFile))
        {
            // pre-2.1: we have to scan the dir to find all matching _x.sn/_x.fn files for our segment
            String prefix;
            if (useCompoundFile)
                prefix = IndexFileNames::segmentFileName(name, IndexFileNames::SEPARATE_NORMS_EXTENSION());
            else
                prefix = IndexFileNames::segmentFileName(name, IndexFileNames::PLAIN_NORMS_EXTENSION());
            int32_t prefixLength = prefix.length();
            HashSet<String> allFiles(dir->listAll());
            for (HashSet<String>::iterator fileName = allFiles.begin(); fileName != allFiles.end(); ++fileName)
            {
                if (IndexFileNameFilter::accept(L"", *fileName) && (int32_t)fileName->length() > prefixLength && 
                    UnicodeUtil::isDigit((*fileName)[prefixLength]) && boost::starts_with(*fileName, prefix))
                    _files.add(*fileName);
            }
        }
        return _files;
    }
    
    void SegmentInfo::clearFiles()
    {
        _files.reset();
        sizeInBytesNoStore = -1;
        sizeInBytesWithStore = -1;
    }
    
    String SegmentInfo::toString()
    {
        return toString(dir, 0);
    }
    
    String SegmentInfo::toString(DirectoryPtr dir, int32_t pendingDelCount)
    {
        StringStream buffer;
        buffer << name << L"(" << (version.empty() ? L"?" : version) << L")" << L":";
        try
        {
            if (getUseCompoundFile())
                buffer << L"c";
            else
                buffer << L"C";
        }
        catch (IOException&)
        {
            buffer << L"?";
        }

        if (this->dir != dir)
            buffer << L"x";
        if (hasVectors)
            buffer << L"v";
        buffer << docCount;

        int32_t delCount = -1;
        try
        {
            delCount = getDelCount();
        }
        catch (IOException&)
        {
        }
        if (delCount != -1)
            delCount += pendingDelCount;
        if (delCount != 0)
        {
            buffer << L"/";
            if (delCount == -1)
                buffer << L"?";
            else
                buffer << delCount;
        }

        if (docStoreOffset != -1)
        {
            buffer << L"->" << docStoreSegment;
            if (docStoreIsCompoundFile)
                buffer << L"c";
            else
                buffer << L"C";
            buffer << L"+" << docStoreOffset;
        }

        return buffer.str();
    }
    
    bool SegmentInfo::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        SegmentInfoPtr otherSegmentInfo(boost::dynamic_pointer_cast<SegmentInfo>(other));
        if (!otherSegmentInfo)
            return false;
        return (otherSegmentInfo->dir == dir && otherSegmentInfo->name == name);
    }
    
    int32_t SegmentInfo::hashCode()
    {
        return dir->hashCode() + StringUtils::hashCode(name);
    }
    
    void SegmentInfo::setVersion(const String& version)
    {
        this->version = version;
    }
    
    String SegmentInfo::getVersion()
    {
        return version;
    }
}
