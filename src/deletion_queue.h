#include <fmt/core.h>
#include <deque>
#include <functional>

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()> &&function) {
		deletors.push_back(function);
	}

	void flush() {
		fmt::println("Flushing deletion queue with {} deletors", deletors.size());
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)();
		}
		deletors.clear();
	}
};
