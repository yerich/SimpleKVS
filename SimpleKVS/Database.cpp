#include "Database.h"

void Collection::set(std::string key, std::string value) {
	if (mWriteBuffers.size() == 0) {
		mWriteBuffers.push_back(new OrderedMap<std::string, std::string>());
	}
	mWriteBuffers.back()->set(key, value);
	mCache.set(key, value);
}

const std::string Collection::get(std::string value) {
	bool isDeleted = false;
	std::string result = mCache.at(value, &isDeleted);
	if (isDeleted) {
		throw std::out_of_range("Key has been deleted");
	}
	return result;
}

void Collection::del(std::string key) {
	mCache.del(key);
	bool isDeleted = mWriteBuffers.back()->del(key);
	if (!isDeleted) {
		mWriteBuffers.back()->set(key, "", true);
	}
}
