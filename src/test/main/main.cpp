/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#endif

#include "TestUtils.h"
#include "MiscUtils.h"
#include "FileUtils.h"
#include "StringUtils.h"

#include "LuceneObject.h"

#define BOOST_TEST_MODULE "Lucene"
#define BOOST_TEST_NO_MAIN

#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/string.hpp>

void freelast();

namespace Lucene
{
    template <typename TYPE> 
    class ArrayTTData
    {
    public:
        ArrayTTData(int32_t size)
        {
            data = NULL;
            resize(size);
        }
        
        ~ArrayTTData()
        {
            resize(0);
        }
    
    public:
        TYPE* data;
        int32_t size;
    
    public:
        void resize(int32_t size)
        {
            if (size == 0)
            {
                FreeMemory(data);
                data = NULL;
            }
            else if (data == NULL)
                data = (TYPE*)AllocMemory(size * sizeof(TYPE));
            else
                data = (TYPE*)ReallocMemory(data, size * sizeof(TYPE));
            this->size = size;
        }
    };
    
    /// Utility template class to handle sharable arrays of simple data types
    template <typename TYPE>
    class ArrayTT : public LuceneSync
    {
    public:
        typedef ArrayTT<TYPE> this_type;
        typedef ArrayTTData<TYPE> array_type;

        ArrayTT()
        {
            array = NULL;
        }
        
        ~ArrayTT()
        {
            array = NULL;
        }
        
    protected:
        LucenePtr<array_type> container;
        array_type* array;
        
    public:
        static this_type newInstance(int32_t size)
        {
            this_type instance;
            instance.container = Lucene::newInstance<array_type>(size);
            instance.array = instance.container.get();
            return instance;
        }
        
        void reset()
        {
            resize(0);
        }
        
        void resize(int32_t size)
        {
            if (size == 0)
                container.reset();
            else if (!container)
                container = Lucene::newInstance<array_type>(size);
            else
                container->resize(size);
            array = container.get();
        }
                
        TYPE* get() const
        {
            return array->data;
        }
        
        int32_t size() const
        {
            return array->size;
        }
        
        bool equals(const this_type& other) const
        {
            if (array->size != other.array->size)
                return false;
            return (std::memcmp(array->data, other.array->data, array->size) == 0);
        }
        
        int32_t hashCode() const
        {
            return (int32_t)(int64_t)array;
        }
        
        TYPE& operator[] (int32_t i) const
        {
            BOOST_ASSERT(i >= 0 && i < array->size);
            return array->data[i];
        }
        
        operator bool () const
        {
            return container;
        }
        
        bool operator! () const
        {
            return !container;
        }
        
        bool operator== (const ArrayTT<TYPE>& other)
        {
            return (container == other.container);
        }
        
        bool operator!= (const ArrayTT<TYPE>& other)
        {
            return (container != other.container);
        }
    };
    
    template <class TYPE>
    inline std::size_t hash_value(const ArrayTT<TYPE>& value)
    {
        return (std::size_t)value.hashCode();
    }

    template <class TYPE>
    inline bool operator== (const ArrayTT<TYPE>& value1, const ArrayTT<TYPE>& value2)
    {
        return (value1.hashCode() == value2.hashCode());
    }
    
    typedef ArrayTT<uint8_t> ByteArrayTT;
}


using namespace Lucene;

static int count =0;

class Tester : public LuceneObject
{
public:
    Tester(int hh)
    {
        gibble = "hello";
        pp = new uint8_t[4096];
        count++;
    }
    
    virtual ~Tester()
    {
        gibble = "boo";
        delete [] pp;
        count--;
    }
    
    LUCENE_CLASS(Tester);

    uint8_t* pp;
    std::string gibble;
};

DECLARE_LUCENE_PTR(Tester)

template < class A >
class TesterTT : public LuceneSync
{
public:
    TesterTT(A hh)
    {
        gibble = "hello";
        pp = new uint8_t[4096];
        count++;
    }
    
    ~TesterTT()
    {
        gibble = "boo";
        delete [] pp;
        count--;
    }
    
    uint8_t* pp;
    std::string gibble;
};

int main(int argc, char* argv[])
{
    //GC_set_dont_precollect(1);
    GC_INIT(); // todo: required?
    
    for (int i = 0; i < 100; ++i)
    {
        //TesterPtr tt = newLucene<Tester>(123);
        //Tester* tt = new(GC) Tester(1);
        
        //TesterTT<int>* tt = new(GC) TesterTT<int>(1);
        
        ByteArrayTT utf8(ByteArrayTT::newInstance(4096));
        
        int hh =3;
    }
    int hhw =3;
    
 
    String testDir;
    uint64_t startTime = MiscUtils::currentTimeMillis();

    for (int32_t i = 0; i < argc; ++i)
    {
        if (strncmp(argv[i], "--test_dir", 9) == 0)
        {
            String testParam = StringUtils::toUnicode(argv[i]);
            Collection<String> vals = StringUtils::split(testParam, L"=");
            if (vals.size() == 2)
            {
                testDir = vals[1];
                boost::replace_all(testDir, L"\"", L"");
                boost::trim(testDir);
                break;
            }
        }
    }
    
    if (testDir.empty())
    {  
        testDir = L"../../src/test/testfiles";
        if (!FileUtils::isDirectory(testDir))
        {
            testDir = L"../src/test/testfiles";
            if (!FileUtils::isDirectory(testDir))
                testDir = L"./src/test/testfiles";
        }
    }
    
    if (!FileUtils::isDirectory(testDir))
    {
        std::wcout << L"Test directory not found. (override default by using --test_dir=\"./src/test/testfiles\")\n";
        return 1;
    }
    
    setTestDir(testDir);

    int testMain = boost::unit_test::unit_test_main(init_unit_test_suite, argc, argv);
    
    std::wcout << L"*** Test duration: " << (MiscUtils::currentTimeMillis() - startTime) / 1000 << L" sec\n";
    
    // GC_gcollect(); // todo
    /* todo while (GC_collect_a_little()) { }
      for (int i = 0; i < 16; i++) {
        GC_gcollect();
        GC_invoke_finalizers();
      }*/
    
    GC_gcollect();
    GC_invoke_finalizers();
        
    //return testMain; todo
    freelast();
    
    return 0;
}
