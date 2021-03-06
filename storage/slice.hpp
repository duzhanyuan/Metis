#pragma once
#ifndef __FL_METIS_STORAGE_SLICE_HPP
#define	__FL_METIS_STORAGE_SLICE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Storage slice management class
///////////////////////////////////////////////////////////////////////////////

#include "file.hpp"
#include <vector>
#include <memory>
#include <limits>
#include <string>
#include "../types.hpp"
#include "bstring.hpp"
#include "buffer.hpp"
#include "exception.hpp"
#include "mutex.hpp"
#include "read_write_lock.hpp"
#include "range_index.hpp"

namespace fl {
	namespace metis {
		using namespace storage;
		
		using fl::fs::File;
		using fl::strings::BString;
		using fl::threads::Mutex;
		using fl::threads::AutoMutex;
		using fl::threads::ReadWriteLock;
		using fl::threads::AutoReadWriteLockRead;
		using fl::threads::AutoReadWriteLockWrite;
		using fl::utils::Buffer;
		
		class SliceError : public fl::exceptions::Error
		{
		public:
			SliceError(const char *what)
				: Error(what)
			{
			}
		};

		static const TSize PACKET_FINISHED_FLAG = 1 << ((sizeof(TSize) * 8) - 1);
		class Slice
		{
		public:
			Slice(const TSliceID sliceID, BString &dataFileName, BString &indexFileName);
			TSize size() const
			{
				return _size;
			}
			bool add(const char *data, IndexEntry &ie);
			bool add(File &putTmpFile, BString &buf, IndexEntry &ie);
			bool get(BString &data, const ItemRequest &item);
			bool get(BString &data, const TItemSize dataSeek, const TItemSize requestSeek, const TItemSize requestSize);
			bool loadIndex(class Index &index, Buffer &buf);
			bool remove(const ItemHeader &ih, const ItemPointer &pointer);
		private:
			void _openDataFile(BString &dataFileName);
			void _openIndexFile(BString &indexFileName);
			void _rebuildIndexFromData(BString &indexFileName);
			bool _writeItem(const char *data, IndexEntry &ie);
			bool _writeItem(File &putTmpFile, BString &buf, IndexEntry &ie);
			struct SliceDataHeader
			{
				static const uint8_t CURRENT_VERSION = 1;
				uint8_t version;
				TSliceID sliceID;
			} __attribute__((packed));

			struct SliceIndexHeader
			{
				static const uint8_t CURRENT_VERSION = 1;
				uint8_t version;
				TSliceID sliceID;
			} __attribute__((packed));
			
			TSliceID _sliceID;
			File _dataFd;
			File _indexFd;
			TSeek _size;
			ReadWriteLock _sync;
		};
		typedef std::shared_ptr<class Slice> TSlicePtr;
		
		class SliceManager
		{
		public:
			SliceManager(const char *path, const double minFree, const TSize maxSliceSize);
			bool add(const char *data, IndexEntry &ie);
			bool add(File &putTmpFile, BString &buf, IndexEntry &ie);
			bool get(BString &data, const ItemRequest &item);
			bool get(BString &data, const ItemPointer &pointer, const TItemSize seek, const TItemSize size);
			bool remove(const ItemHeader &ih, const ItemPointer &pointer);
			bool loadIndex(class Index &index);
			bool findWriteSlice(const TItemSize size);
			bool ping(StoragePingAnswer &storageAnswer);
		private:
			void _formDataPath(BString &path);
			void _formIndexPath(BString &path);
			void _init();
			bool _addWriteSlice(const TSize size);
			bool _recalcSpace();
			std::string _path;
			double _minFree;
			int64_t _leftSpace;
			TSize _maxSliceSize;
			
			typedef std::vector<TSlicePtr> TSliceVector;
			TSliceVector _slices;
			TSlicePtr _writeSlice;
			Mutex _sync;
		};
	};
};

#endif	// __FL_METIS_STORAGE_SLICE_HPP
