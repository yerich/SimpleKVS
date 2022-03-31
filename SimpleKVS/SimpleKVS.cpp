#include <iostream>
#include "OrderedMap.h"

int main() {
	OrderedMap<int, int> test = OrderedMap<int, int>(50);
	DEBUG_MODE = true;
	test.set(10, 20);
	test.set(20, 20);
	test.set(30, 20);
	test.set(40, 20);
	test.set(50, 20);
	test.set(60, 20);
	test.set(70, 20);
	test.set(80, 20);
	test.set(90, 20);
	test.set(55, 20);
	test.set(56, 20);
	test.set(57, 20);
	test.set(58, 20);
	test.set(59, 20);
	test.set(8, 20);
	test.set(9, 20);
	test.set(7, 20);
	test.set(57, 4444);
	test.print();
	for (auto&& [key, value, _] : test) {
		std::cout << key << ": " << value << std::endl;
	}
	return 0;
}