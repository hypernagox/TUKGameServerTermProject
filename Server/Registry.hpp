#pragma once
#include "pch.h"

#include <string>
#include <vector>
#include <map>

template <typename T>
class Registry
{
private:
	std::vector<std::shared_ptr<T>> id_container;
	std::unordered_map<std::wstring, std::shared_ptr<T>> key_container;

public:
	Registry();
	~Registry();

	size_t Insert(const std::wstring& key, T* item);
	size_t Insert(const std::wstring& key, std::shared_ptr<T>item);

	T* operator[](const int index) const;
	T* operator[](std::wstring_view key) const;
};

template<typename T>
inline Registry<T>::Registry()
{
	id_container.reserve(100);
}

template<typename T>
inline Registry<T>::~Registry()
{
	
}

template<typename T>
inline size_t Registry<T>::Insert(const std::wstring& key, T* item)
{
	std::shared_ptr<T> wrapper{item};
	id_container.push_back(wrapper);
	key_container.insert(make_pair(key, wrapper));

	return id_container.size() - 1;
}

template<typename T>
inline size_t Registry<T>::Insert(const std::wstring& key, std::shared_ptr<T> item)
{
	id_container.push_back(item);
	key_container.insert(make_pair(key, item));

	return id_container.size() - 1;
}

template<typename T>
inline T* Registry<T>::operator[](int index) const
{
	return id_container[index].get();
}

template<typename T>
inline T* Registry<T>::operator[](std::wstring_view key) const
{
	return key_container.find(key.data())->second.get();
}
