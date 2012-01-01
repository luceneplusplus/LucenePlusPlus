/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef VIANTUTILS_H
#define VIANTUTILS_H

#include <boost/any.hpp>
#include "Lucene.h"
#include "MiscUtils.h"

namespace Lucene
{
    // todo: go through these "static" classes (UnicodeUtils, StringUtils, TestPoint, FileUtils, VariableUtils)
    // todo: do they really need to be derived from gc_object?
    // todo: instead we could make the destructors private
    class LPPAPI VariantUtils : public gc_object
    {
    public:
        template <typename T>
        static T get(boost::any var)
        {
            return var.type() == typeid(T) ? boost::any_cast<T>(var) : T();
        }

        template <typename T, typename V>
        static T get(V var)
        {
            return var.type() == typeid(T) ? boost::get<T>(var) : T();
        }

        template <typename T, typename V>
        static bool typeOf(V var)
        {
            return (var.type() == typeid(T));
        }

        static VariantNull null()
        {
            return VariantNull();
        }

        static bool isNull(boost::any var)
        {
            return var.empty();
        }

        template <typename V>
        static bool isNull(V var)
        {
            return typeOf<VariantNull>(var);
        }

        template <typename V>
        static int32_t hashCode(V var)
        {
            if (typeOf<String>(var))
                return StringUtils::hashCode(get<String>(var));
            if (typeOf<int32_t>(var))
                return get<int32_t>(var);
            if (typeOf<int64_t>(var))
                return (int32_t)get<int64_t>(var);
            if (typeOf<double>(var))
            {
                int64_t longBits = MiscUtils::doubleToLongBits(get<double>(var));
                return (int32_t)(longBits ^ (longBits >> 32));
            }
            if (typeOf< Collection<uint8_t> >(var))
                return get< Collection<uint8_t> >(var).hashCode();
            if (typeOf< Collection<int32_t> >(var))
                return get< Collection<int32_t> >(var).hashCode();
            if (typeOf< Collection<int64_t> >(var))
                return get< Collection<int64_t> >(var).hashCode();
            if (typeOf< Collection<double> >(var))
                return get< Collection<double> >(var).hashCode();
            if (typeOf< Collection<String> >(var))
                return get< Collection<String> >(var).hashCode();
            if (typeOf<LuceneObjectPtr>(var))
                return get<LuceneObjectPtr>(var)->hashCode();
            return 0;
        }

        template <typename FIRST, typename SECOND>
        static bool equalsType(FIRST first, SECOND second)
        {
            return (first.type() == second.type());
        }

        template <typename FIRST, typename SECOND>
        static bool equals(FIRST first, SECOND second)
        {
            return first.type() == second.type() ? (first == second) : false;
        }

        template <typename V>
        static int32_t compareTo(V first, V second)
        {
            return first < second ? -1 : (first == second ? 0 : 1);
        }
    };
}

#endif
