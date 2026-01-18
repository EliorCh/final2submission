#pragma once

template <typename T>
T* getItemAt(std::vector<T>& list, const Point& p) { //'T' is deduced automatically based on the vector passed
	for (auto& item : list) {
		if (item.getPos() == p) return &item;
	}
	return nullptr; // item wasn't found
}

template <typename T>
// delete template func using iterator for going through the vector list
bool removeItemAt(std::vector<T>& list, const Point& p) {
	for (size_t i = 0; i < list.size(); ++i) {
		if (list[i].getPos() == p) {
			if (i != list.size() - 1) {
				list[i] = std::move(list.back());
		}
		list.pop_back();
		return true;
		}
	}
	return false;
}