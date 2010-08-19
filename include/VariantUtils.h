/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lucene.h"

namespace Lucene
{
	class LPPAPI VariantUtils
	{
	public:
		template <typename TYPE, typename VAR>
		static TYPE get(VAR var)
		{
			return var.type() == typeid(TYPE) ? boost::get<TYPE>(var) : TYPE();
		}
		
		template <typename TYPE, typename VAR>
		static bool typeOf(VAR var)
		{
			return (var.type() == typeid(TYPE));
		}
		
		template <typename VAR>
		static bool isBlank(VAR var)
		{
			return typeOf<Blank>(var);
		}
		
		template <typename VAR>
		static int32_t hashCode(VAR var)
		{
			if (typeOf<String>(var))
				return StringUtils::hashCode(boost::get<String>(var));
			if (typeOf<int32_t>(var))
				return boost::get<int32_t>(var);
			if (typeOf<int64_t>(var))
				return (int32_t)boost::get<int64_t>(var);
			if (typeOf<double>(var))
			{
				int64_t longBits = MiscUtils::doubleToLongBits(boost::get<double>(var));
				return (int32_t)(longBits ^ (longBits >> 32));
			}
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
		
		template <typename VAR>
		static int32_t compareTo(VAR first, VAR second)
		{
			return first < second ? -1 : (first == second ? 0 : 1);
		}
	};
}
