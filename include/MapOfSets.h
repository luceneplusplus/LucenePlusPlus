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
    template <class MAPKEY, class MAPHASH, class MAPEQUAL, class SETVALUE, class SETHASH, class SETEQUAL>
    class MapOfSets : public LuceneSync
    {
    public:
        typedef HashSet<SETVALUE, SETHASH, SETEQUAL> set_type;
        typedef HashMap<MAPKEY, set_type, MAPHASH, MAPEQUAL> map_type;

        MapOfSets(map_type m)
        {
            theMap = m;
        }

    protected:
        map_type theMap;

    protected:
        virtual void mark_members(gc* gc) const
        {
            gc->mark(theMap);
            LuceneSync::mark_members(gc);
        }

    public:
        /// @return direct access to the map backing this object.
        map_type getMap()
        {
            return theMap;
        }

        /// Adds val to the HashSet associated with key in the HashMap.  If key is not already in the map,
        /// a new HashSet will first be created.
        /// @return the size of the HashSet associated with key once val is added to it.
        int32_t put(MAPKEY key, SETVALUE val)
        {
            typename map_type::iterator entry = theMap.find(key);
            if (entry != theMap.end())
            {
                entry->second.add(val);
                return entry->second.size();
            }
            else
            {
                set_type theSet(set_type::newInstance());
                theSet.add(val);
                theMap.put(key, theSet);
                return 1;
            }

            // todo
            // std::pair<MAP::iterator, bool> entry = theMap.insert(std::make_pair(key, SET()));
            // entry.first->second.insert(val);
            // return entry.first->second.size();
        }

        /// Adds multiple vals to the HashSet associated with key in the HashMap.  If key is not already in
        /// the map, a new HashSet will first be created.
        /// @return the size of the HashSet associated with key once val is added to it.
        int32_t put(MAPKEY key, set_type vals)
        {
            typename map_type::iterator entry = theMap.find(key);
            if (entry != theMap.end())
            {
                entry->second.add(vals.begin(), vals.end());
                return entry->second.size();
            }
            else
            {
                set_type theSet(set_type::newInstance(vals.begin(), vals.end()));
                theMap.put(key, theSet);
                return theSet.size();
            }

            // todo
            // std::pair<MAP::iterator, bool> entry = theMap.insert(std::make_pair(key, SET()));
            // entry.first->second.insert(vals.begin(), vals.end());
            // return entry.first->second.size();
        }
    };
}

#endif
