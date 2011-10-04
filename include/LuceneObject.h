/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENEOBJECT_H
#define LUCENEOBJECT_H

#ifndef LPP_USE_GC
#include <boost/enable_shared_from_this.hpp>
#endif
#include "LuceneSync.h"

#ifdef LPP_USE_CYCLIC_CHECK
#define LUCENE_INTERFACE(Name) \
    static String _getClassName() { return L###Name; } \
    virtual String getClassName() { return L###Name; } \
    CycleCheckT<Name> cycleCheck;
#else
#define LUCENE_INTERFACE(Name) \
    static String _getClassName() { return L###Name; } \
    virtual String getClassName() { return L###Name; }
#endif

#ifdef LPP_USE_GC
#define LUCENE_CLASS(Name) \
    LUCENE_INTERFACE(Name); \
    LucenePtr<Name> LuceneThis() { return LucenePtr<Name>(this); }
#else
#define LUCENE_CLASS(Name) \
    LUCENE_INTERFACE(Name); \
    LucenePtr<Name> LuceneThis() { return LuceneStaticCast<Name>(LuceneObject::shared_from_this()); }
#endif

// todo: to tidy up the code we could define LuceneObject as: class LPPAPI LuceneObject : public LuceneSync, gc_cleanup
// and remove this from LuceneLync (probably not too bothered about calling the destructor for LuceneSync)

namespace Lucene
{
    /// Base class for all Lucene classes
    #ifdef LPP_USE_GC
    class LPPAPI LuceneObject : public LuceneSync
    #else
    class LPPAPI LuceneObject : public LuceneSync, public boost::enable_shared_from_this<LuceneObject>
    #endif
    {
    public:
        virtual ~LuceneObject();
    
    protected:
        LuceneObject();

    public:
        /// Called directly after instantiation to create objects that depend on this object being 
        /// fully constructed.
        virtual void initialize();
        
        /// Return clone of this object
        /// @param other clone reference - null when called initially, then set in top virtual override.
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        /// Return hash code for this object.
        virtual int32_t hashCode();
        
        /// Return whether two objects are equal
        virtual bool equals(LuceneObjectPtr other);
        
        /// Compare two objects
        virtual int32_t compareTo(LuceneObjectPtr other);
        
        /// Returns a string representation of the object
        virtual String toString();
    };
}

#endif
