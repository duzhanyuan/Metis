#pragma once
#ifndef __FL_METIS_MANAGER_CACHE_HPP
#define	__FL_METIS_MANAGER_CACHE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Metis's manager cache system classes
///////////////////////////////////////////////////////////////////////////////


#include "config.h"
#ifdef HAVE_CXX11
	#include <unordered_map>
	#include <unordered_set>
	using std::unordered_map;
	using std::unordered_set;
#else
	#include <boost/unordered_map.hpp>
	#include <boost/unordered_set.hpp>
	using boost::unordered_map;
	using boost::unordered_set;
#endif
	
#include <memory>
#include <set>
#include "mutex.hpp"
#include "cluster_manager.hpp"
#include "bstring.hpp"
#include "../types.hpp"

namespace fl {
	namespace metis {
		using fl::threads::Mutex;
		using fl::threads::AutoMutex;
		using fl::strings::BString;
		
		enum class ECacheFindResult
		{
			NOT_IN_CACHE,
			FIND_NOT_FOUND,
			FIND_FULL,
			FIND_HEADER_ONLY,
		};

		typedef uint16_t TCacheLineIndex;
		typedef uint16_t TCacheHits;
		class ItemCache
		{
		public:
			void update(const GetItemInfoAnswer &item, const TStorageList &storages, size_t &freedMem);
			void set(TCacheLineIndex cacheIndex, const GetItemInfoAnswer &item, const TStorageList &storages);
			size_t free();
			const TCacheLineIndex cacheIndex() const
			{
				return _cacheIndex;
			}
			bool replaceData(const GetItemInfoAnswer &item, const char *data, int64_t &usedMem, const uint32_t minHitsToCache);
			ItemCache()
				: _data(NULL)
			{
			}
			~ItemCache();
			void hit(TItemSize &size, ModTimeTag &modTimeTag);
			bool fillBuffer(BString &buffer);
			bool haveData() const
			{
				return _data != NULL;
			}
			TCacheHits hits() const
			{
				return _hits;
			}
			TItemSize size() const
			{
				return _size;
			}
		private:
			static const uint8_t MAX_STORAGES = 3;
			TCacheLineIndex _cacheIndex;
			TCacheHits _hits;
			TItemSize _size;
			ModTimeTag _timeTag;		
			StorageNode *_nodes[MAX_STORAGES];
			char *_data;
		};
		
		struct ItemIndexHash : std::unary_function<ItemIndex, std::size_t>
		{
			size_t operator()(const fl::metis::ItemIndex &ie) const
			{
				size_t key = ie.rangeID;
				key <<= (sizeof(key) * 8 - 32);
				key |= ie.itemKey;
				return key;
			};
		};

		class CacheLine
		{
		public:
			CacheLine()
			{
			}
			void resize(const TCacheLineIndex countItemInfoItems, const uint32_t minHitsToCache);
			bool replace(const GetItemInfoAnswer &item, const TStorageList &storages, size_t &freedMem);
			bool replaceData(const GetItemInfoAnswer &item, const char *data, int64_t &usedMem);
			ECacheFindResult find(const TRangeID rangeID, const TItemKey itemKey, TItemSize &size, ModTimeTag &modTimeTag,
				BString &buffer);
			bool remove(const ItemIndex &itemIndex, size_t &freedMem);
			size_t recycle(const size_t needFree);
		private:

			typedef unordered_map<ItemIndex, ItemCache*, ItemIndexHash> TItemCacheMap;
			TItemCacheMap _items;
			
			typedef std::vector<ItemCache> TItemCacheVector;
			TItemCacheVector _itemsCache;
			
			typedef std::vector<TCacheLineIndex> TFreeItemCacheIndexVector;
			TFreeItemCacheIndexVector _freeIndexes;		
			
			typedef std::set<ItemIndex> TNotFoundItemsSet;
			TNotFoundItemsSet _notFoundItems;
			Mutex _sync;
			
			void _free(ItemCache *ic, size_t &freedMem);
			uint32_t _minHitsToCache;
		};
		
		class Cache
		{
		public:
			Cache(const size_t cacheSize, const size_t itemHeadersCacheSize, const TCacheLineIndex itemsInLine, 
				const uint32_t minHitsToCache);
			bool replace(const GetItemInfoAnswer &item, const TStorageList &storages);
			bool remove(const ItemIndex &itemIndex);
			bool replaceData(const GetItemInfoAnswer &item, const char *data);
			
			ECacheFindResult find(const TRangeID rangeID, const TItemKey itemKey, TItemSize &size, ModTimeTag &modTimeTag,
				BString &buffer);
			
			void recycle();
			int64_t leftMem()
			{
				return _leftMem;
			}
			size_t countLines() 
			{
				return _lines.size();
			}
		private:
			typedef std::vector<CacheLine> TCacheLineVector;
			TCacheLineVector _lines;
			
			int64_t _leftMem;
			size_t _minFreeMem;
		};
	};
};

#endif	// __FL_METIS_MANAGER_CACHE_HPP