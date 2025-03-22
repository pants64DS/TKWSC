#ifndef FIXED_SIZE_ACTOR_LIST
#define FIXED_SIZE_ACTOR_LIST

#include <algorithm>
#include "SM64DS_PI.h"

template<std::size_t maxSize>
class FixedSizeActorList
{
	std::size_t size = 0;
	std::array<unsigned, maxSize> uniqueIDs;

public:
	friend const ostream& operator<<(const ostream& os, const FixedSizeActorList& l)
	{
		os << '[';

		if (l.size > 0)
		{
			for (unsigned i = 0; i < l.size - 1; ++i)
				os << l.uniqueIDs[i] << ", ";

			os << l.uniqueIDs[l.size - 1];
		}

		return os << ']';
	}

	void Insert(unsigned newUniqueID)
	{
		if (size == maxSize)
			return;

		const auto begin = uniqueIDs.begin();
		const auto end = begin + size;

		const auto insertionPos = std::lower_bound(begin, end, newUniqueID);

		if (insertionPos != end && *insertionPos == newUniqueID)
			return; // The list already contains the unique ID

		std::shift_right(insertionPos, end + 1, 1);

		*insertionPos = newUniqueID;
		++size;
	}

	void ForEach(auto&& f)
	{
		if (size == 0) return;

		unsigned i = 0;
		Actor::ListNode* node = FIRST_ACTOR_LIST_NODE;

		while (true)
		{
			if (!node)
			{
				size = i;

				break;
			}
			else if (uniqueIDs[i] < node->actor->uniqueID)
			{
				std::shift_left(&uniqueIDs[i], &uniqueIDs[size], 1);

				if (--size <= i) break;
			}
			else
			{
				if (uniqueIDs[i] == node->actor->uniqueID)
				{
					f(*node->actor);

					if (++i == size) break;
				}

				node = node->next;
			}
		}
	}

	void Clear()
	{
		size = 0;
	}
};

#endif