#ifndef SRC_RANGE_WALK
#define SRC_RANGE_WALK
template<class T>
class RangeInt {
	T current;
	T target;
	bool direction;//true -> forward |  false -> backward
public:
	RangeInt(T begin, T end) {
		current = begin;
		target = end;
		direction = begin < end;
	}
	T& operator*() {
		return current;
	}
	operator T() {
		return current;
	}
	RangeInt& operator++(int) {
		if (direction)
			++current;
		else
			--current;
		return *this;
	}
	RangeInt operator++() {
		RangeInt res = *this;
		if (direction)
			++current;
		else
			--current;
		return res;
	}
	bool operator==(RangeInt ri) {
		return ri.target == target && ri.current == current;
	}
	bool operator!=(RangeInt ri) {
		return !operator==(ri);
	}

	bool operator<(RangeInt& ri) {
		return current < ri.target;
	}
	bool operator>(RangeInt& ri) {
		return current > ri.target;
	}
	RangeInt& begin() {
		return *this;
	}
	RangeInt end() {
		return RangeInt(target, target);
	}
};


#endif /* SRC_RANGE_WALK */
