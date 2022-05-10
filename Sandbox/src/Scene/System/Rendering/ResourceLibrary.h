#pragma once
#include "D3DCore.h"

#include <unordered_map>
#include <string>

template <class T>
class ResourceLibrary
{
public:
	static void Add(const std::string& key, const std::shared_ptr<T>& resource)
	{
		D3DC_ASSERT(m_lib.find(key) == m_lib.end(), "Storing duplicate");
		m_lib[key] = resource;
	}

	static std::shared_ptr<T> Get(const std::string& key)
	{
		D3DC_ASSERT(m_lib.find(key) != m_lib.end(), "Resource does not exists");
		return m_lib[key];
	}

	static void Remove(const std::string& key)
	{
		D3DC_ASSERT(m_lib.find(key) != m_lib.end(), "Resource does not exists");
		m_lib.erase(key);
	}

	static void Clear()
	{
		m_lib.clear();
	}

	//std::unordered_map<std::string, std::shared_ptr<T>>::iterator begin() { m_lib.begin(); }
	//std::unordered_map<std::string, std::shared_ptr<T>>::iterator end() { m_lib.end(); }
	//std::unordered_map<std::string, std::shared_ptr<T>>::reverse_iterator rbegin() { m_lib.rbegin(); }
	//std::unordered_map<std::string, std::shared_ptr<T>>::reverse_iterator rend() { m_lib.rend(); }

	//std::unordered_map<std::string, std::shared_ptr<T>>::const_iterator begin() const { m_lib.begin(); }
	//std::unordered_map<std::string, std::shared_ptr<T>>::const_iterator end() const { m_lib.end(); }
	//std::unordered_map<std::string, std::shared_ptr<T>>::const_reverse_iterator rbegin() const { m_lib.rbegin(); }
	//std::unordered_map<std::string, std::shared_ptr<T>>::const_reverse_iterator rend() const { m_lib.rend(); }

private:
	static inline std::unordered_map<std::string, std::shared_ptr<T>> m_lib;
};