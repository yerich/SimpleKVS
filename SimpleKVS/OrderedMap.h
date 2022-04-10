#pragma once
#include <concepts>
#include <iostream>
#include <stdexcept>
#include <math.h>

constexpr size_t DEFAULT_BRANCHING_FACTOR = 3;

template<typename K, typename V>
	requires std::totally_ordered<K>
class OrderedMapNode;

template<typename V>
struct OrderedMapNodeValue {
	bool isDeleted;
	V value;
};

template<typename K, typename V>
	requires std::totally_ordered<K>
union OrderedMapNodeChild {
	OrderedMapNode<K, V>* node;
	OrderedMapNodeValue<V> value;
	OrderedMapNodeChild() : node{ nullptr } {};
	OrderedMapNodeChild(OrderedMapNode<K, V>* node) : node{ node } {};
	OrderedMapNodeChild(OrderedMapNodeValue<V> value) : value{ value } {};
	OrderedMapNodeChild(const OrderedMapNodeChild<K, V>& other) { memcpy(this, &other, sizeof(OrderedMapNodeChild<K, V>)); };
	OrderedMapNodeChild& operator=(const OrderedMapNodeChild& other)
	{
		memcpy(this, &other, sizeof(OrderedMapNodeChild<K, V>));
		return *this;
	};
	~OrderedMapNodeChild() {};
};

template<typename K, typename V>
	requires std::totally_ordered<K>
class OrderedMapNode {
	// A node in the B+ tree
	// Note: for branch nodes, the first key is used to store the minimum value
	// contained within the subtree.
public:
	size_t mSize;
	bool mIsLeafNode;
	OrderedMapNodeChild<K, V>* mChildren;
	K* mKeys;
	OrderedMapNode* mNext;

	OrderedMapNode(size_t branchingFactor=DEFAULT_BRANCHING_FACTOR) : 
		mSize{ 0 }, 
		mIsLeafNode{ true },
		mKeys{ new K[branchingFactor] },
		mChildren{ new OrderedMapNodeChild<K, V>[branchingFactor] },
		mNext { nullptr }
	{};
	OrderedMapNode(const OrderedMapNode& other) = delete;
	OrderedMapNode(OrderedMapNode&& other) :
		mSize{ 0 },
		mIsLeafNode{ true },
		mKeys{ nullptr },
		mNext{ nullptr }
	{
		swap(*this, other);
	}
	OrderedMapNode& operator=(const OrderedMapNode& other) = delete;
	OrderedMapNode& operator=(OrderedMapNode&& other)
	{
		swap(*this, other);
		return *this;
	};

	friend void swap(OrderedMapNode& a, OrderedMapNode& b)
	{
		using std::swap;
		std::swap(a.mSize, b.mSize);
		std::swap(a.mChildren, b.mChildren);
		std::swap(a.mIsLeafNode, b.mIsLeafNode);
		std::swap(a.mChildrenKeys, b.mChildrenKeys);
		std::swap(a.mNext, b.mNext);
	}

	OrderedMapNode* set_value(K key, OrderedMapNodeChild<K, V> value, size_t branchingFactor);

	void print(std::ostream& out = std::cout, int depth = 0)
	{
		if (depth == 0) {
			out << "== BEGIN TREE ==" << std::endl;
		}
		if (mIsLeafNode) {
			for (size_t i = 0; i < mSize; i++) {
				for (int j = 0; j < depth; j++) out << "    ";
				out << mKeys[i] << ": " << mChildren[i].value.value << std::endl;
			}
		}
		else {
			for (size_t i = 0; i < mSize; i++) {
				for (int j = 0; j < depth; j++) out << "    ";
				if (i == 0)
					out << " (" << mKeys[i] << ")" << std::endl;
				else
					out << "> " << mKeys[i] << std::endl;
				mChildren[i].node->print(out, depth + 1);
			}
		}
		if (depth == 0) {
			out << "== END TREE ==" << std::endl;
		}
	}

	// For a given key, return the index of the subtree that would contain the key.
	size_t child_position(K key) const {
		if (mSize <= 1 || key < mKeys[1]) return 0;

		ptrdiff_t left = 1;
		ptrdiff_t right = mSize - 1;
		ptrdiff_t mid;
		while (right > left) {
			mid = (left + right + 1) / 2;
			if (key < mKeys[mid]) {
				right = mid - 1;
			}
			else {
				left = mid;
			}
		}
		return left;  
	}

	K min_key() {
		if (mIsLeafNode) {
			return mKeys[0];
		}
		return mChildren[0].node->min_key();
	}

	~OrderedMapNode() {
		if (!mIsLeafNode) {
			for (size_t i = 0; i < mSize; i++) {
				delete mChildren[i].node;
			}
		}
		else {
			for (size_t i = 0; i < mSize; i++) {
				mChildren[i].value.OrderedMapNodeValue<V>::~OrderedMapNodeValue();
			}
		}
		delete[] mChildren;
		delete[] mKeys;
	};
};


// A map with linear-time key-order traversal
// Implemented as a B+ tree
template<typename K, typename V>
	requires std::totally_ordered<K>
class OrderedMap
{
public:
	struct Iterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = struct { K first; V second; bool isDeleted; };
		using pointer = value_type*;
		using reference = value_type&; 

		Iterator(K key, OrderedMapNodeValue<V> value, const OrderedMapNode<K, V>* parent) :
			ptr{ value_type { key, value.value, value.isDeleted } },
			parent { parent },
			keyIndex { 0 }
		{
			if (!parent) return;
			for (size_t i = 0; i < parent->mSize; i++) {
				if (parent->mKeys[i] == key) {
					keyIndex = i;
				}
			}
		};
		reference operator*() { return ptr; }
		pointer operator->() { return &ptr; }

		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
		Iterator& operator++() 
		{ 
			if (keyIndex < parent->mSize - 1) {
				keyIndex += 1;
			}
			else if (parent->mNext) {
				parent = parent->mNext;
				keyIndex = 0;
			}
			else {
				parent = nullptr;
				return *this;
			}
			ptr.first = parent->mKeys[keyIndex];
			ptr.second = parent->mChildren[keyIndex].value.value;
			return *this; 
		}

		friend bool operator== (const Iterator& a, const Iterator& b) 
		{
			if (a.parent == nullptr && b.parent == nullptr) return true;
			return a.parent == b.parent && a.keyIndex == b.keyIndex;
		};
		friend bool operator!= (const Iterator& a, const Iterator& b) 
		{
			if (a.parent == nullptr && b.parent == nullptr) return false;
			return a.parent != b.parent || a.keyIndex != b.keyIndex;
		};

		value_type ptr;
		const OrderedMapNode<K, V>* parent;
		size_t keyIndex;
	};

	OrderedMap(size_t branchingFactor=DEFAULT_BRANCHING_FACTOR) :
		mRoot{ new OrderedMapNode<K, V>(branchingFactor) },
		mBranchingFactor{ branchingFactor },
		mHeight { 1 }
	{};
	OrderedMap(const OrderedMap& other) = delete;
	OrderedMap(OrderedMap&& other) noexcept : OrderedMap()
	{
		swap(*this, other);
	};
	OrderedMap& operator=(const OrderedMap& other) = delete;
	OrderedMap& operator=(OrderedMap&& other)
	{
		swap(*this, other);
		return *this;
	}
	~OrderedMap() {
		delete mRoot;
	};

	friend void swap(OrderedMap& a, OrderedMap& b)
	{
		using std::swap;
		swap(a.mRoot, b.mRoot);
		swap(a.mBranchingFactor, b.mBranchingFactor);
	};

	Iterator begin() const 
	{
		if (mRoot->mSize == 0) return end();
		auto curr = mRoot;
		while (!curr->mIsLeafNode) curr = curr->mChildren[0].node;
		return Iterator(curr->mKeys[0], curr->mChildren[0].value, curr);
	}

	Iterator end() const
	{
		return Iterator(mRoot->mKeys[mRoot->mSize], mRoot->mChildren[mRoot->mSize].value, nullptr);
	}

	V& at(K key, bool* isDeleted = nullptr) const 
	{
		auto curr = mRoot;
		while (true) {
			if (!curr || curr->mSize == 0) throw std::out_of_range("Key not found");
			if (!curr->mIsLeafNode) {
				size_t i = curr->child_position(key);
				curr = curr->mChildren[i].node;
			}
			else {
				ptrdiff_t left = 0;
				ptrdiff_t right = curr->mSize - 1;
				ptrdiff_t mid;

				while (left <= right) {
					mid = (left + right) / 2;
					if (key == curr->mKeys[mid]) {
						if (isDeleted) *isDeleted = curr->mChildren[mid].value.isDeleted;
						return curr->mChildren[mid].value.value;
					}
					else if (key < curr->mKeys[mid]) {
						right = mid - 1;
					}
					else {
						left = mid + 1;
					}
				}
				throw std::out_of_range("Key not found");
			}
		}
	}

	bool del(K key)
	{
		auto curr = mRoot;
		while (true) {
			if (!curr || curr->mSize == 0) return false;
			if (!curr->mIsLeafNode) {
				size_t i = curr->child_position(key);
				curr = curr->mChildren[i].node;
			}
			else {
				ptrdiff_t left = 0;
				ptrdiff_t right = curr->mSize - 1;
				ptrdiff_t mid;

				while (left <= right) {
					mid = (left + right) / 2;
					if (key == curr->mKeys[mid]) {
						curr->mChildren[mid].value.isDeleted = true;
						return true;
					}
					else if (key < curr->mKeys[mid]) {
						right = mid - 1;
					}
					else {
						left = mid + 1;
					}
				}
				return false;
			} 
		}
	}

	void set(K key, V value, bool setAsDeleted = false); 
	void print(std::ostream& out = std::cout) const
	{
		mRoot->print(out);
	}

	V& operator[](const K& key) const {}
private:
	OrderedMapNode<K, V>* mRoot;
	size_t mBranchingFactor;
	size_t mHeight;
};

template<typename K, typename V>
	requires std::totally_ordered<K>
OrderedMapNode<K, V>* OrderedMapNode<K, V>::set_value(K key, OrderedMapNodeChild<K, V> value, size_t branchingFactor)
{
	// linear-time insertion
	// First determine the insertion point
	size_t i;
	for (i = 0; i < mSize && mKeys[i] < key; i++);
	if (mIsLeafNode && i < mSize && mKeys[i] == key) {
		// Value exists, set it
		mChildren[i] = value;
		return nullptr;
	}

	if ((mIsLeafNode && mSize == branchingFactor - 1) || (!mIsLeafNode && mSize == branchingFactor)) {
		// Split the node into two new nodes, return the new node
		size_t half = (size_t)ceil((double)mSize / 2.0f);

		OrderedMapNode* newNode = new OrderedMapNode(branchingFactor);
		newNode->mIsLeafNode = mIsLeafNode;

		size_t k = 0;

		if (i < half) {
			// The new value will get inserted in the first half
			// First, copy the second half over to the new node
			for (size_t j = half - 1; j < mSize; j++, k++) {
				newNode->mKeys[k] = mKeys[j];
				newNode->mChildren[k] = mChildren[j];
				if (!mIsLeafNode) mChildren[j].node = nullptr;
			}
			if (half >= 2) {
				// Shift down the necessary portion of the first half (which is our current node)
				for (size_t j = half - 2; j >= i; j--) {
					mKeys[j + 1] = mKeys[j];
					mChildren[j + 1] = mChildren[j];
					if (j == 0) break; // prevent negative integer overflow
				}
			}
			// Insert the new value in the first half
			mKeys[i] = key;
			mChildren[i] = value;

			newNode->mSize = k;
		}
		else {
			// The new value will get inserted in the second half
			for (size_t j = half; j < i; j++, k++) {
				// Copy over the part of the second half that comes before the value
				newNode->mKeys[k] = mKeys[j];
				newNode->mChildren[k] = mChildren[j];
				if (!mIsLeafNode) mChildren[j].node = nullptr;
			}
			// Add the new value to the new node
			newNode->mKeys[k] = key;
			newNode->mChildren[k] = value;
			k++;
			// Copy the remainder of the second half
			for (size_t j = i; j < mSize; j++, k++) {
				newNode->mKeys[k] = mKeys[j];
				newNode->mChildren[k] = mChildren[j];
				if (!mIsLeafNode) mChildren[j].node = nullptr;
			}

			newNode->mSize = k;
		}

		if (mIsLeafNode) {
			newNode->mNext = mNext;
			mNext = newNode;
		}

		mSize = half;
		return newNode;
	}
	else {
		// No split
		if (mSize > 0) {
			for (size_t j = mSize - 1; j >= i; j--) {
				mKeys[j + 1] = mKeys[j];
				mChildren[j + 1] = mChildren[j];
				if (j == 0) break;	// prevent negative integer overflow
			}
		}
		mKeys[i] = key;
		mChildren[i] = value;
		mSize++;
	}

	return nullptr;
}


template<typename K, typename V>
	requires std::totally_ordered<K>
void OrderedMap<K, V>::set(K key, V value, bool setAsDeleted)
{
	OrderedMapNodeChild<K, V> newValue = OrderedMapNodeChild<K, V>( { .isDeleted = setAsDeleted, .value = value });

	OrderedMapNode<K, V>** stack = new OrderedMapNode<K, V>*[mHeight+1];

	stack[0] = mRoot;
	size_t i = 0;
	OrderedMapNode<K, V>* curr = mRoot;

	while (!curr->mIsLeafNode) {
		i += 1;
		size_t j = curr->child_position(key);

		curr->mKeys[j] = std::min(curr->mKeys[j], key);
		curr = curr->mChildren[j].node;
		stack[i] = curr;
	};

	OrderedMapNode<K, V>* newNode = curr->set_value(key, newValue, mBranchingFactor);

	if (!newNode) {
		delete[] stack;
		return;
	}

	while (i > 0 && newNode) {
		i -= 1;
		OrderedMapNode<K, V>* parent = stack[i];
		K newKey = newNode->min_key();
		OrderedMapNodeChild<K, V> newValue = OrderedMapNodeChild<K, V>(newNode);
		newNode = parent->set_value(newKey, newValue, mBranchingFactor);
	}

	if (i == 0 && newNode) {
		OrderedMapNode<K, V>* newRoot = new OrderedMapNode<K, V>(mBranchingFactor);
		newRoot->mSize = 2;
		newRoot->mIsLeafNode = false;
		newRoot->mChildren[0].node = mRoot;
		newRoot->mChildren[1].node = newNode;
		newRoot->mKeys[0] = mRoot->min_key();
		newRoot->mKeys[1] = newNode->min_key();

		if (mRoot->mIsLeafNode)
			mRoot->mNext = newNode;

		mRoot = newRoot;
		mHeight++;
	}

	delete[] stack;
}