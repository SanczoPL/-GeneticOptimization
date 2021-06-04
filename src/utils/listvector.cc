#include "utils/listvector.h"


template<typename T>
ListVector<T>::ListVector()
{
  Logger->trace("ListVector::ListVector()");
  m_size = 20;
  T _t;
  for (int i = 0; i < m_size; i++)
  {
	m_vect.push_back(_t);
	m_active.push_back(false);
  }

  Logger->trace("size m_vect:{}", m_vect.size());
}

template<typename T>
ListVector<T>::ListVector(int size)
{
  Logger->trace("ListVector::ListVector()");
  m_size = size;
  T _t;
  for (int i = 0; i < size; i++)
  {
	m_vect.push_back(_t);
	m_active.push_back(false);
  }

  Logger->trace("size m_vect:{}", m_vect.size());
}

template<typename T>
bool ListVector<T>::isActive(int id)
{
  if (m_active[id])
  {
	return true;
  }
  else
  {
	return false;
  }
}

template<typename T>
int ListVector<T>::addItem(T data)
{
	
  for (int i = 0; i < m_size; i++)
  {
	if (!m_active[i])
	{
		m_active[i] = true;
		m_vect[i] = data;
		Logger->trace("ListVector<T>::addItem() success");
		return i;
	}
  }	
  Logger->warn("ListVector<T>::addItem() failed");
  return -1;
}

template<typename T>
bool ListVector<T>::addItem(int id, T data)
{
	if(id < 0)
	{
		Logger->warn("ListVector<T>::addItem() failed");
		return false;
	}
	if (m_active[id])
	{
		m_vect[id] = data;
		Logger->trace("ListVector<T>::addItem() success");
		return true;
	}
	Logger->warn("ListVector<T>::addItem() failed");
	return false;
}

template<typename T>
T& ListVector<T>::operator[] (int index)
{
	Logger->trace("ListVector<T>::operator[] index:{}", index);
	if(index < m_size && index >= 0 && m_active[index])
	{
		return m_vect.at(index);
	}
	Logger->error("ListVector<T>::operator[] index out of bound");
	throw std::out_of_range( "operator[] index out of bound" );
}

template<typename T>
int ListVector<T>::leaseItem()
{
	for (int i = 0; i < m_size; i++)
	{
		if (!m_active[i])
		{
			m_active[i] = true;
			Logger->trace("ListVector<T>::leaseItem() lease {} id", i);
			return i;
		}
	}
	Logger->warn("ListVector<T>::leaseItem() lease failed");
	return -1;
}

template<typename T>
bool ListVector<T>::deleteItem(int id)
{
	if (id >= 0 && id < m_size)
	{
		if (m_active[id])
		{
			m_active[id] = false;
			Logger->trace("ListVector<T>::deleteItem() success");
			return true;
		}
	}
	Logger->warn("ListVector<T>::deleteItem() failed");
	return false;
}

template<typename T>
bool ListVector<T>::deleteItems()
{
	for(int i = 0 ; i < m_size ; i++)
	{
		if (m_active[i])
		{
			m_active[i] = false;
		}
	}
	Logger->warn("ListVector<T>::deleteItem() failed");
	return true;
}

template<typename T>
int ListVector<T>::size()
{
	int counter{0};
	for(int i = 0 ; i < m_size ; i++)
	{
		if (m_active[i])
		{
			counter+=1;
		}
	}
	return counter;
}

template class ListVector<int>;
