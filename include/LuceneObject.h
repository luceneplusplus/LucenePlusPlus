/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENEOBJECT_H
#define LUCENEOBJECT_H

#include "LuceneSync.h"

// todo: is this required?
#define LUCENE_INTERFACE(Name) \
    static String _getClassName() { return L###Name; } \
    virtual String getClassName() { return L###Name; }

// todo: can we simple use "this" instead of LuceneThis()?
#define LUCENE_CLASS(Name) \
    LUCENE_INTERFACE(Name); \
    gc_ptr<Name> LuceneThis() { return gc_ptr<Name>(this); }

namespace Lucene
{
    /// Base class for all Lucene classes
    class LPPAPI LuceneObject : public LuceneSync
    {
    public:
        virtual ~LuceneObject();

    protected:
        LuceneObject();

    public:
        /// Called directly after instantiation to create objects that depend on this object being
        /// fully constructed.
        // todo: is this required now (restriction on shared_ptr gone!)
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
