#ifndef __WS_OBJECT_POOL_H__
#define __WS_OBJECT_POOL_H__

#include <list>
#include <mutex>
#include "utils/Log.h"

namespace ws
{
	template<class T>
	class ObjectPool
	{
	public:
		ObjectPool<T>() : capacity(0xFFFFFFFF) {}
		ObjectPool<T>(size_t nCapacity) : capacity(nCapacity) {}

		~ObjectPool<T>()
		{
			for (T* element : list)
			{
				delete element;
			}
		}

		T* alloc()
		{
			T* pT = nullptr;
			if (list.size() > 0)
			{
				pT = list.back();
				list.pop_back();
			}
			else
			{
				pT = new T;
			}
			return pT;
		}

		void free(T* pT)
		{
			if (list.size() < capacity)
			{
				list.push_back(pT);
			}
			else
			{
				Log::w("object pool is full");
			}
		}

		typename std::list<T*>::iterator begin()
		{
			return list.begin();
		}

		typename std::list<T*>::iterator end()
		{
			return list.end();
		}

		size_t			size() { return list.size(); }
		void			lock() { mtx.lock(); }
		void			unlock() { mtx.unlock(); }

	private:
		std::list<T*>	list;
		size_t			capacity;
		std::mutex		mtx;
	};
}
#endif