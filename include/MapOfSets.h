/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MAPOFSETS_H
#define MAPOFSETS_H

#include "Lucene.h"

namespace Lucene
{
    /// Helper class for keeping Lists of Objects associated with keys.
    template <class MAP, class SET>
    class MapOfSets : public LuceneSync
    {
    public:
        MapOfSets(MAP m)
        {
            theMap = m;
        }

    protected:
        MAP theMap;

    public:
        /// @return direct access to the map backing this object.
        MAP getMap()
        {
            return theMap;
        }

        /// Adds val to the HashSet associated with key in the HashMap.  If key is not already in the map,
        /// a new HashSet will first be created.
        /// @return the size of the HashSet associated with key once val is added to it.
        int32_t put(MAP::key_type key, SET::value_type val)
        {
            std::pair<MAP::iterator, bool> entry = theMap.insert(std::make_pair(key, SET()));
            entry.first->second.insert(val);
            return entry.first->second.size();
        }

        /// Adds multiple vals to the HashSet associated with key in the HashMap.  If key is not already in
        /// the map, a new HashSet will first be created.
        /// @return the size of the HashSet associated with key once val is added to it.
        int32_t put(MAP::key_type key, SET vals)
        {
            std::pair<MAP::iterator, bool> entry = theMap.insert(std::make_pair(key, SET()));
            entry.first->second.insert(vals.begin(), vals.end());
            return entry.first->second.size();
        }
    };
}

#endif
