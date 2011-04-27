/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#endif

#include "Lucene.h"
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/crc.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/function.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/any.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>

