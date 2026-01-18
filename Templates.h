#pragma once

//learned by ourselves when saw too much duplicates of the same funcs
template <typename T> //means the next func isn't a reg func, it's a template
T* getItemAt(std::vector<T>& list, const Point& p) { //'T' is deduced automatically based on the vector passed
	for (auto& item : list) {

		if (item.getPos() == p) return &item;
	}
	return nullptr; //item wasn't found
}

template <typename T>
// delete template func using iterator for going through the vector list
bool removeItemAt(std::vector<T>& list, const Point& p) {
	for (auto it = list.begin(); it != list.end(); ++it) {
		if (it->getPos() == p) {
			*it = std::move(list.back()); // Swap & Pop
			list.pop_back();
			return true;
		}
	}
	return false;
}
