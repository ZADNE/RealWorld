#pragma once
#include <limits>

template<class T> class DestroyTicket {
	friend class WorldDrawer;
public:
	DestroyTicket<T>() : m_ID(std::numeric_limits<decltype(m_ID)>::max()) {};

	bool operator<(const DestroyTicket<T> otherTicket) const {//Ordering tickets in map
		return m_ID < otherTicket.m_ID;
	};
private:
	DestroyTicket<T>(size_t ID) : m_ID(ID) {};
	size_t m_ID;
};