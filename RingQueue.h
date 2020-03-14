#ifndef _RING_QUEUE_H_
#define _RING_QUEUE_H_

#include "MacroDef.h"

template <typename T, int SIZE>
class ring_queue
{
private:
	T _queue[SIZE];
	int _head_idx; // get from here
	int _tail_idx; // put to here

public:
	ring_queue() :
		_head_idx = 0,
		_tail_idx = 0
	{
	}

	void clear()
	{
		_head_idx = 0;
		_tail_idx = 0;
	}

	bool empty()
	{
		return _head_idx == _tail_idx;
	}

	int len()
	{
		return ((_tail_idx - _head_idx) + SIZE) % SIZE;
	}

	/*
	void set(T& value)
	{
	_queue[_head_idx] = T;

	if (_head_idx == _tail_idx)
	{
	_tail_idx = (_tail_idx + 1) % SIZE;
	}
	}
	*/

	void put(T& value) // put value to tail
	{
		_queue[_tail_idx] = T;
		_tail_idx = (_tail_idx + 1) % SIZE;

		if (_tail_idx == _head_idx) // full, discard first
		{
			_head_idx = (_head_idx + 1) % SIZE;
		}
	}

	T get() // get value from head and discard
	{
		T ret; // need default constructer for empty

		if (_head_idx != _tail_idx)
		{
			ret = _queue[_head_idx];
			_head_idx = (_head_idx + 1) % SIZE;
		}

		return ret;
	}

	void discard() // discard head
	{
		if (_tail_idx != _head_idx)
		{
			_head_idx = (_head_idx + 1) % SIZE;
		}
	}

	T* head() // head address
	{
		if (_head_idx != _tail_idx)
		{
			return &_queue[_head_idx]
		}
		else
		{
			return nullptr;
		}
	}

	T* tail() // tail address
	{
		if (_head_idx != _tail_idx)
		{
			return &_queue[_tail_idx]
		}
		else
		{
			return nullptr;
		}
	}
};

#endif //_RING_QUEUE_H_