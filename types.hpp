#pragma once
#ifndef __FL_METIS_TYPES_HPP
#define	__FL_METIS_TYPES_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Metis internal types
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <vector>
#include "socket.hpp"

namespace fl {
	namespace metis
	{
		typedef uint32_t TServerID;
		
		typedef uint8_t TItemStatus;
		static const TItemStatus ST_ITEM_DELETED = 0x80;
		
		typedef uint8_t TManagerStatus;
		typedef uint8_t TStorageStatus;
		typedef uint32_t TStorageGroupID;
		
		typedef uint32_t TLevel;
		typedef uint32_t TSubLevel;
		typedef uint32_t TRangeID;
		typedef uint32_t TItemKey;
		typedef uint32_t TItemSize;
		typedef uint32_t TItemModTime;
		
		typedef uint32_t TCrc;
		
		typedef uint16_t TCacheLineIndex;

		union ModTimeTag
		{
			struct
			{
				uint32_t op;
				TItemModTime modTime;
			};
			uint64_t tag;
			bool operator <=(const ModTimeTag &tag) const
			{
				if (modTime < tag.modTime)
					return true;
				else if (modTime == tag.modTime)
					return op <= tag.op;
				else
					return false;
			}
		};

		struct ItemHeader
		{
			TItemStatus status; // must be first field in a ItemHeader structure
			TRangeID rangeID;
			TLevel level;
			TSubLevel subLevel;
			TItemKey itemKey;
			TItemSize size;
			ModTimeTag timeTag;
		}  __attribute__((packed));
		
		typedef uint32_t TSeek;
		typedef uint32_t TSize;
		typedef uint16_t TSliceID;
		
		struct ItemPointer
		{
			TSliceID sliceID;
			TSeek seek;
		} __attribute__((packed));
		
		struct ItemRequest
		{
			ItemPointer pointer;
			TSize size;
		} __attribute__((packed));

		struct IndexEntry
		{
			ItemHeader header;
			ItemPointer pointer;
		} __attribute__((packed));	
		
		enum EStorageCMD : uint8_t
		{
			STORAGE_NO_CMD = 0,
			STORAGE_ITEM_INFO,
			STORAGE_PUT,
			STORAGE_GET_ITEM_CHUNK,
			STORAGE_DELETE_ITEM,
			STORAGE_PING,
			STORAGE_GET_RANGE_ITEMS,
			STORAGE_SYNC,
		};
		
		struct StorageCmd
		{
			EStorageCMD cmd;
			TSize size;
		} __attribute__((packed));
		
		enum EStorageAnswerStatus : uint8_t
		{
			STORAGE_NO_ANSWER = 0,
			STORAGE_ANSWER_OK,
			STORAGE_ANSWER_ERROR,
			STORAGE_ANSWER_NOT_FOUND,
		};
		struct StorageAnswer
		{
			EStorageAnswerStatus status;
			TSize size;
		} __attribute__((packed));
		
		struct ItemIndex
		{
			ItemIndex(const TRangeID rangeID = 0, const TItemKey itemKey = 0)
				: rangeID(rangeID), itemKey(itemKey)
			{
			}
			bool operator==(const ItemIndex &index) const
			{
				if (rangeID != index.rangeID)
					return false;
				if (itemKey == index.itemKey)
					return true;
				else
					return false;
			}
			bool operator<(const ItemIndex &index) const
			{
				if (rangeID < index.rangeID)
					return true;
				else if (rangeID == index.rangeID)
					return (itemKey < index.itemKey);
				else
					return false;
			}
			TRangeID rangeID;
			TItemKey itemKey;
		} __attribute__((packed));
		
		struct ItemLevelIndex
		{
			ItemLevelIndex()
				: level(0), subLevel(0), itemKey(0)
			{
			}
			ItemLevelIndex(const ItemHeader &itemHeader) 
				: level(itemHeader.level), subLevel(itemHeader.subLevel), itemKey(itemHeader.itemKey)
			{
			}
			TLevel level;
			TSubLevel subLevel;
			TItemKey itemKey;	
		};
		
		struct ItemInfo
		{
			ItemInfo()
				: size(0)
			{
				timeTag.tag = 0;
			}
			ItemInfo(const ItemHeader &itemHeader)
				: index(itemHeader.rangeID, itemHeader.itemKey), size(itemHeader.size), timeTag(itemHeader.timeTag)
			{
			}
			ItemIndex index;
			TItemSize size;
			ModTimeTag timeTag;
		} __attribute__((packed));
		
		struct GetItemChunkRequest
		{
			TRangeID rangeID;
			TItemKey itemKey;
			TItemSize chunkSize;
			TItemSize seek;
		} __attribute__((packed));
		
		struct StoragePingAnswer
		{
			TServerID serverID;
			int64_t leftSpace;
		} __attribute__((packed));
		
		struct RangeItemsRequest
		{
			TServerID serverID;
			TRangeID rangeID;
		}__attribute__((packed));
		
		struct RangeItemsHeader
		{
			TRangeID rangeID;
			uint32_t count;
		} __attribute__((packed));
		
		struct RangeItemEntry
		{
			TItemKey itemKey;
			TSize size;
			ModTimeTag timeTag;
		} __attribute__((packed));
		
		struct RangeSyncHeader
		{
			TServerID managerID;
			TRangeID rangeID;
			uint32_t count;
		} __attribute__((packed));
		
		struct RangeSyncEntry
		{
			ItemHeader header;
			TServerID fromServer;
			fl::network::TIPv4 ip;
			uint32_t port;
		} __attribute__((packed));
		
		typedef std::vector<RangeSyncEntry> TIndexSyncEntryVector;
	};
};

#endif	// __FL_METIS_TYPES_HPP
