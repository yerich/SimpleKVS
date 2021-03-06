#include "pch.h"
#include "CppUnitTest.h"
#include "../SimpleKVS/OrderedMap.h"
#include <vector>
#include <algorithm>
#include <set>
#include <random>
#include <map>

#include "windows.h"
#define _CRTDBG_MAP_ALLOC //to get more details
#include <stdlib.h>  
#include <crtdbg.h>   //for malloc and free


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

long lrand()
{
	return static_cast<long>(rand()) << (sizeof(int) * 8) | rand();

	return rand();
}

namespace UnitTest
{
	TEST_CLASS(UnitTest)
	{
	public:
		
		TEST_METHOD(SimpleTree)
		{
			OrderedMap<int, int> test = OrderedMap<int, int>(3);

			std::vector<int> keys = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 55, 56, 57, 58, 59, 8, 9, 7 };

			for (auto k : keys) {
				test.set(k, k);
			}

			for (auto k : keys) {
				Assert::AreEqual(k, test.at(k));
			}

			std::sort(keys.begin(), keys.end());
			validate_ordered_map(test, keys);
		}

		TEST_METHOD(RandomTrees)
		{
			for (size_t iteration = 0; iteration < 100; iteration++) {
				srand(iteration);
				OrderedMap<long, long> test = OrderedMap<long, long>(iteration + 3);

				long size = 1000;
				std::vector<long> keys;
				keys.reserve(size);

				for (long i = 0; i < size; i++) {
					keys.push_back(i - size / 2);
				}
				std::shuffle(keys.begin(), keys.end(), std::default_random_engine(iteration));
				for (auto k : keys) {
					test.set(k, k);
				}

				std::sort(keys.begin(), keys.end());
				validate_ordered_map(test, keys);
			}
		}

		TEST_METHOD(VeryLarge)
		{
			OrderedMap<int64_t, int64_t> test = OrderedMap<int64_t, int64_t>(100);

			int64_t size = 1'000'000;

			for (int64_t i = 0; i < size; i++) {
				test.set(i, i);
			}

			for (int64_t key : { 0, 12442, 45987, 166234, 621115 }) {
				Assert::AreEqual(test.at(key), key);
			}

			int64_t acc = 0;
			for (auto const& [key, value, _] : test) {
				acc += key;
			}

			Assert::AreEqual(acc, (int64_t)499999500000);
			Assert::AreEqual(test.at(size-1), size-1);
		}

		TEST_METHOD(VeryLargeMap)
		{
			std::map<int64_t, int64_t> test = std::map<int64_t, int64_t>();

			int64_t size = 1'000'000;

			for (int64_t i = 0; i < size; i++) {
				test[i] = i;
			}

			for (int64_t key : { 0, 12442, 45987, 166234, 621115 }) {
				Assert::AreEqual(test.at(key), key);
			}

			int64_t acc = 0;
			for (auto const& [key, value] : test) {
				acc += key;
			}

			Assert::AreEqual(acc, (int64_t)499999500000);
			Assert::AreEqual(test.at(0), (int64_t) 0);
			Assert::AreEqual(test.at(size - 1), size - 1);
		}

		TEST_METHOD(LowerBound)
		{
			OrderedMap<int64_t, int64_t> test = OrderedMap<int64_t, int64_t>(25);
			std::vector<int> keys = { 10, 20, 30, 40, 50, 60 };
			// n.b. keys[0] shouldn't matter for this algorithm

			for (auto k : keys) {
				test.set(k, k);
			}
			auto root = test.begin().parent;
			
			Assert::AreEqual(root->child_position(0), (size_t)0);
			Assert::AreEqual(root->child_position(9), (size_t)0);
			Assert::AreEqual(root->child_position(10), (size_t)0);
			Assert::AreEqual(root->child_position(20), (size_t)1);
			Assert::AreEqual(root->child_position(30), (size_t)2);
			Assert::AreEqual(root->child_position(39), (size_t)2);
			Assert::AreEqual(root->child_position(59), (size_t)4);
			Assert::AreEqual(root->child_position(60), (size_t)5);
			Assert::AreEqual(root->child_position(100), (size_t)5);
		}

		TEST_METHOD(DeleteValue)
		{
			OrderedMap<int, int> test = OrderedMap<int, int>(3);

			std::vector<int> keys = { 1, 2, 3, 4, 5, 6, 7 };

			for (auto k : keys) {
				test.set(k, k);
			}

			for (auto k : keys) {
				Assert::AreEqual(k, test.at(k));
			}

			Assert::IsTrue(test.del(3));
			Assert::IsFalse(test.del(300));

			bool isDeleted = false;
			Assert::AreEqual(3, test.at(3, &isDeleted));
			Assert::IsTrue(isDeleted);

			test.set(3, 3);
			Assert::AreEqual(3, test.at(3, &isDeleted));
			Assert::IsFalse(isDeleted);
		}

		TEST_METHOD(DeletedValueIterator)
		{
			OrderedMap<int, int> test = OrderedMap<int, int>(3);

			std::vector<int> keys = { 1, 2, 3, 4, 5, 6, 7 };

			for (auto k : keys) {
				test.set(k, k);
			}
			
			for (auto const& [key, value, isDeleted] : test) {
				Assert::AreEqual(key, value);
				Assert::IsFalse(isDeleted);
			}

			std::ostringstream output_buffer;
			output_buffer << keys.size() << std::endl;
			test.print(output_buffer);
			Logger::WriteMessage(output_buffer.str().c_str());

			for (auto k : keys) {
				Assert::IsTrue(test.del(k));
			}

			for (auto const& [key, value, isDeleted] : test) {
				Assert::AreEqual(key, value);
				Assert::IsTrue(isDeleted);
			}
		}

		TEST_METHOD(Strings)
		{
			OrderedMap test = OrderedMap<int64_t, std::string>(100);

			int64_t size = 1000;

			for (int64_t i = 0; i < size; i++) {
				test.set(i, std::to_string(i));
			}

			for (int64_t key : { 0, 12, 24, 35 }) {
				Assert::AreEqual(test.at(key), std::to_string(key));
			}
			Assert::AreEqual(test.at(0), std::to_string(0));
		}

		TEST_METHOD(OutOfBounds)
		{
			OrderedMap test = OrderedMap<int64_t, std::string>(10);
			test.set(10, "Hello");
			Assert::ExpectException<std::out_of_range>([&test] { test.at(12); });
		}

		TEST_METHOD(MemoryLeak)
		{
			_CrtMemState sOld;
			_CrtMemState sNew;
			_CrtMemState sDiff;
			_CrtMemCheckpoint(&sOld); //take a snapshot

			auto* test = new OrderedMap<int64_t, std::string>(10);

			int64_t size = 10000;

			for (int64_t i = 0; i < size; i++) {
				test->set(i, "hello");
			}

			delete test;

			_CrtMemCheckpoint(&sNew); //take a snapshot 
			Assert::IsFalse(_CrtMemDifference(&sDiff, &sOld, &sNew));
			/*
			if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
			{
				OutputDebugString(L"-----------_CrtMemDumpStatistics ---------");
				_CrtMemDumpStatistics(&sDiff);
				OutputDebugString(L"-----------_CrtMemDumpAllObjectsSince ---------");
				_CrtMemDumpAllObjectsSince(&sOld);
				OutputDebugString(L"-----------_CrtDumpMemoryLeaks ---------");
				_CrtDumpMemoryLeaks();
			}
			*/
		}
	private:
		template<typename K, typename V>
		void validate_ordered_map(const OrderedMap<K, V>& map, const std::vector<K>& keys)
		{
			std::ostringstream output_buffer;
			//output_buffer << keys.size() << std::endl;
			//map.print(output_buffer);
			size_t i = 0;
			for (auto const& [key, value, _] : map) {
				//output_buffer << key << std::endl;
				Assert::AreEqual(key, value);
				Assert::AreEqual(key, keys[i]);
				i += 1;
			}

			Assert::AreEqual(i, keys.size());
			Logger::WriteMessage(output_buffer.str().c_str());
		}
	};
}
