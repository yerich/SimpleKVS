#pragma once
#include <unordered_map>
#include "OrderedMap.h"

class Collection
{
public:
	Collection(std::string name = "") :
		mName { name },
		mCache(100)	// TODO determine buffer size based on system cache size
	{}
	Collection(const Collection& other) = delete;
	Collection(Collection&& other) noexcept : Collection(other.mName)
	{
		swap(*this, other);
	};
	Collection& operator=(const Collection& other) = delete;
	Collection& operator=(Collection&& other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	~Collection() {
		for (auto b : mWriteBuffers) {
			delete b;
		}
	};

	friend void swap(Collection& a, Collection& b)
	{
		using std::swap;
		swap(a.mName, b.mName);
		swap(a.mWriteBuffers, b.mWriteBuffers);
		swap(a.mCache, b.mCache);
	};

	std::string name() const { return mName; }

	void set(std::string key, std::string value);
	const std::string get(std::string value);
	void del(std::string key);
private:
	std::string mName;
	std::vector<OrderedMap<std::string, std::string>*> mWriteBuffers;
	OrderedMap<std::string, std::string> mCache;
};

class Database
{
public:
	Collection& addCollection(std::string collectionName) {
		if (mCollections.contains(collectionName)) {
			return mCollections[collectionName];
		}
		mCollections[collectionName] = Collection(collectionName);
		return mCollections[collectionName];
	}

	Collection& getCollection(std::string collectionName) {
		return mCollections.at(collectionName);
	}
private:
	std::unordered_map<std::string, Collection> mCollections;
};

