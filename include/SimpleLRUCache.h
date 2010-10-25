/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lucene.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp> 
#include <boost/multi_index/sequenced_index.hpp>

namespace Lucene
{
	/// General purpose LRU cache map.
	/// Accessing an entry will keep the entry cached.  {@link #get(const KEY&, VALUE&)} and
	/// {@link #put(const KEY&, const VALUE&)} results in an access to the corresponding entry.
	template <typename KEY, typename VALUE>
	class SimpleLRUCache
	{
	public:
		typedef std::pair<KEY, VALUE> cache_item;
		typedef boost::multi_index::multi_index_container< cache_item, boost::multi_index::indexed_by<
                boost::multi_index::hashed_unique< boost::multi_index::member<cache_item, KEY, &cache_item::first> >,
                boost::multi_index::sequenced<> >, Allocator<cache_item> > cache_items;
		typedef typename cache_items::iterator iterator;
		typedef typename cache_items::template nth_index<0>::type hash_index;
		typedef typename hash_index::iterator hash_index_iterator;
		typedef typename cache_items::template nth_index<1>::type sequenced_index;
		typedef typename sequenced_index::iterator sequenced_index_iterator;
		
		SimpleLRUCache(int32_t cacheSize)
		{
			this->cacheSize = cacheSize;
		}
		
		virtual ~SimpleLRUCache()
		{
		}
	
	protected:
		cache_items cacheItems;
		int32_t cacheSize;
	
	public:
		int32_t size()
		{
			return cacheItems.size();
		}
		
		bool get(const KEY& key, VALUE& value)
		{
			hash_index& hashIndex = cacheItems.template get<0>();
			hash_index_iterator it1 = hashIndex.find(key);
            
			if (it1 == hashIndex.end())
				return false;
			value = it1->second;

			sequenced_index& sequencedIndex = cacheItems.template get<1>();
			sequencedIndex.relocate(sequencedIndex.end(), cacheItems.template project<1>(it1));

			return true;
		}

		void put(const KEY& key, const VALUE& value)
		{
			if (cacheSize > 0 && (int32_t)cacheItems.size() >= cacheSize)
			{
				sequenced_index& sequencedIndex = cacheItems.template get<1>();
				sequencedIndex.erase(sequencedIndex.begin());
			}

			hash_index& hashIndex = cacheItems.template get<0>();
			hash_index_iterator it = hashIndex.find(key);

			if (it == hashIndex.end())
				cacheItems.insert(std::make_pair(key, value));
			else
				hashIndex.replace(it, std::make_pair(key, value));
		}

		iterator begin()
		{
			return cacheItems.begin();
		}
		
		iterator end()
		{
			return cacheItems.end();
		}
	};
};

