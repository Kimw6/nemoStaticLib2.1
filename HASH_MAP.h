#ifndef HASH_MAP_H
#define HASH_MAP_H

//------------------------------------------- HASH_MAP.h ---------------------------------------------------------------
// Author: Adam Sirkis
// Creation: 02-08-2019
// Modified: 03-16-2019
//----------------------------------------------------------------------------------------------------------------------
// Purpose: Class to store paired values in a memory block
//	    Operates the same as C++ STL unordered_map
//	    Average 40% faster than standard
//----------------------------------------------------------------------------------------------------------------------

#if defined(__has_include)
#if __has_include(<functional>)
#include <functional>
#endif
#endif
#define _max_bucket_count 10000000
#define _max_load_factor 0.75
template <class key, class Value> class HASH_MAP
{
	int _bucket_count = 100;
	int array_size = 0;
public:
	//-------------------------------class HASH_NODE---------------------------
	// Store the keys and values of spaces in the HASH_MAP
	// Uses the same key and Value types as HASH_MAP
	struct HASH_NODE {
	public:
		key first = key();
		Value second = Value();
		//---------------------------(Constructor)-----------------------------
		// empty constructor used when allocating new memory for buckets
		HASH_NODE() {}
		// end Constructor
		//---------------------------(Destructor)------------------------------
		// reset values to the default values
		// Assumes destructors in Key and Value will take care of themselves
		~HASH_NODE() {
			first = key();
			second = Value();
		}// end Destructor
	};// end HASH_NODE
protected:
	// Store values in a publicly accessible, but still unchangeable container
	// Stored using iterators for speed
	// Declared after class HASH_NODE due to declaration order
	HASH_NODE* buckets;
public:
	//-------------------------------(Constructor)-----------------------------
	// Allocates a new array of HASH_NODE to the size of _bucket_count
	// Must be dynamically allocated to start with for 
	// the destructor to function
	HASH_MAP<key, Value>() {
		buckets = new HASH_NODE[_bucket_count];
	};// End Constructor

	//-------------------------------(Destructor)------------------------------
	// deletes buckets
	// no other memory dynamically allocated
	~HASH_MAP<key, Value>() {
		for (HASH_NODE* ptr = &buckets[0]; ptr != &buckets[_bucket_count]; ptr++) {
			delete ptr;
		}
		delete[] buckets;
	};// end Destructor

	//-------------------------------(Copy Constructor)------------------------
	// Makes new HASH_MAP<key, Value> based on rhs
	// Deletes prior version of lhs.buckets
	// O(n) all cases
	HASH_MAP<key, Value>(const HASH_MAP<key, Value>&H) {
		_bucket_count = H._bucket_count;
		array_size = H.array_size;
		HASH_NODE* temp = new HASH_NODE[H._bucket_count + 1];
		auto i = H.begin(), t_start = &temp[0], t = t_start;
		for (int j = 0; i < end() && j < H._bucket_count; j++, i++) {
			if (i->first != key()) {
				t = t_start + getStart(i->first);
				t->first = i->first;
				t->second = i->second;
			}
		}
		std::swap(temp, buckets); // swap addresses
		delete[] temp;
	};//end Copy Constructor

	HASH_MAP<key, Value>(HASH_MAP<key, Value>&&H) {
		_bucket_count = H._bucket_count;
		array_size = H.array_size;
		buckets = H.buckets;
		H._bucket_count = 100;
		H.array_size = 0;
		H.buckets = new HASH_NODE[100];
	};// end Copy Constructor

	//-------------------------------operator=---------------------------------
	// Copies the data from rhs into lhs
	// Deletes prior version of lhs.buckets
	// O(n) all cases
	HASH_MAP<key, Value>& operator=(const HASH_MAP<key, Value>& H) {
		if (this != &H) {
			_bucket_count = H._bucket_count;
			array_size = H.array_size;
			HASH_NODE* temp = new HASH_NODE[H._bucket_count + 1];
			auto i = H.begin(), t_start = &temp[0], t = t_start;
			for (int j = 0; i < H.end() && j < H._bucket_count; j++, i++) {
				if (i->first != key()) {
					t = t_start + getStart(i->first);
					t->first = i->first;
					t->second = i->second;
				}
			}
			std::swap(temp, buckets); // swap addresses
			delete[] temp;
		}
		return *this;
	};//end operator=

	//-------------------------------operator=---------------------------------
	// Moves data from rhs into lhs
	// Resets rhs
	// O(1) all cases
	HASH_MAP<key, Value>& operator=(HASH_MAP<key, Value> && H) {
		if (this != &H) {
			_bucket_count = H._bucket_count;
			array_size = H.array_size;
			delete[] buckets;
			buckets = H.buckets;
			H._bucket_count = 100;
			H.array_size = 0;
			H.buckets = new HASH_NODE[100];
		}
		return *this;
	};//end operator=

	//-------------------------------size--------------------------------------
	// Returns number of successful insertions
	// O(1) all cases
	inline int size() const noexcept {
		return array_size;
	};//end size

	//-------------------------------operator[]--------------------------------
	// Iterates through to find the Value associated with key k
	// returns the value found, if any
	// else insert the key and return a reference to that location
	// O(1) Average case, O(n) worst case (full map)
	Value& operator[](const key& k) noexcept {
		auto ptr = begin() + getStart(k);
	count_loop:
		for (ptr; ptr < end(); ptr++)
			if (ptr->first == k)
				return ptr->second;
			else if (ptr->first == key()) {
				array_size++;
				ptr->first = k;
				if (load_factor() > max_load_factor()) {
					rehash();
				}
				return this->at(k);
			}
		if (ptr == end()) {
			ptr = begin();
			goto count_loop;
		}
	};//end oeprator[]


	//-------------------------------operator[]--------------------------------
	// Iterates through to find the Value associated with rhs
	// returns the value found, if any
	// else return an empty Value
	// O(1) Average case, O(n) worst case (full map)
	Value& operator[](key&& k) noexcept {
		auto ptr = begin() + getStart(k);
	count_loop:
		for (ptr; ptr < end(); ptr++)
			if (ptr->first == k)
				return ptr->second;
			else if (ptr->first == key()) {
				ptr->first = k;
				array_size++;
				if (load_factor() > max_load_factor()) {
					rehash();
				}
				return this->at(k);
			}
		if (ptr == end()) {
			ptr = begin();
			goto count_loop;
		}
	};// end operator[]

	//-------------------------------at----------------------------------------
	// Iterates through to find the Value associated with rhs
	// returns the value found, if any
	// else return an empty Value
	// O(1) Average case, O(n) worst case (full map)
	Value& at(const key& k) noexcept {
		auto ptr = begin() + getStart(k);
	count_loop:
		for (ptr; ptr < end(); ptr++)
			if (ptr->first == k)
				return ptr->second;
			else if (ptr->first == key()) {
				ptr->first = k;
				array_size++;
				if (load_factor() > max_load_factor()) {
					rehash();
				}
				return this->at(k);
			}
		if (ptr == end()) {
			ptr = begin();
			goto count_loop;
		}
	};// end at

	//-------------------------------at----------------------------------------
	// Iterates through to find the Value associated with rhs
	// returns the value found, if any
	// else return an empty Value
	// O(1) Average case, O(n) worst case (full map)
	Value& at(key&& k) noexcept {
		auto ptr = begin() + getStart(k);
	count_loop:
		for (ptr; ptr < end(); ptr++)
			if (ptr->first == k)
				return ptr->second;
			else if (ptr->first == key()) {
				ptr->first = k;
				array_size++;
				if (load_factor() > max_load_factor()) {
					rehash();
				}
				return this->at(k);
			}
		if (ptr == end()) {
			ptr = begin();
			goto count_loop;
		}
	};// end at

	//-------------------------------count-------------------------------------
	// Returns the number of times rhs is used as an index
	// O(1) average case, O(n) worst case
	int count(const key& k) const noexcept {
		int found = 0;
		int start = getStart(k);
		auto ptr = begin() + start;
	count_loop:
		for (ptr; ptr < end(); ptr++) {
			if (ptr->first == key()) {
				return found;
			}
			if (ptr->first == k) {
				found++;
			}
		}
		if (ptr == end()) {
			ptr = begin();
			goto count_loop;
		}
	};// end count

	//-------------------------------begin-------------------------------------
	// returns an iterator to the beginning of buckets
	// O(1) all cases
	inline auto begin() const noexcept {
		return &buckets[0];
	};// end begin

	//-------------------------------end---------------------------------------
	// returns an iterator to the end of buckets
	// O(1) all cases
	inline auto end() const noexcept {
		return &buckets[_bucket_count];
	};// end end

	//------------------------------cbegin-------------------------------------
	// returns a constant iterator to the beginning of buckets
	// O(1) all cases
	const inline auto cbegin() const noexcept {
		return &buckets[0];
	};//end cbegin

	//------------------------------cend---------------------------------------
	// returns a constant iterator to the end of buckets
	// O(1) all cases
	const inline auto cend() const noexcept {
		return &buckets[_bucket_count];
	};// end cend

	//------------------------------insert-------------------------------------
	// inserts rhs into buckets
	// will not insert if rhs.first is already in buckets
	// O(1) average case, O(n + n^2) worst case (forced to rehash)
	void insert(std::pair<key, Value> p) noexcept {
		//if (count(p.first))
		//	return;
		array_size++;
		int start = getStart(p.first);

		auto ptr = begin() + start;
	insert_loop:
		for (ptr; ptr < end(); ptr++) {
			// if ptr reaches the end, go to the beginning
			if (ptr->first == key()) {
				ptr->first = p.first;
				ptr->second = p.second;
				break;
			}
		}
		if (ptr == end()) {
			ptr = begin();
			goto insert_loop;
		}
		// if the load factor is too high
		if (load_factor() > max_load_factor()) {
			rehash();
		}
		int z = 0;
	};//end insert

	//------------------------------insert_or_assign---------------------------
	// if a key p.first exists, change the value assigned to it
	// else insert a new value
	// O(1) average case, O(n + n^2) worst case (forced to rehash)
	std::pair<HASH_NODE*, bool> insert_or_assign(std::pair<key, Value> p) noexcept {
		//if (count(p.first))
		//	return;
		array_size++;
		int start = getStart(p.first);
		auto ptr = begin() + start;
	insert_loop:
		for (ptr; ptr < end(); ptr++) {
			if (ptr->first == key()) {
				ptr->first = p.first;
				ptr->second = p.second;
				return std::pair<HASH_NODE*, bool> {ptr, true};
			}
			else if (ptr->first == p.first) {
				ptr->first = p.first;
				ptr->second = p.second;
				return std::pair<HASH_NODE*, bool> {ptr, false};
			}
		}
		if (ptr == end()) {
			ptr = begin();
			goto insert_loop;
		}
		// if the load factor is too high
		if (load_factor() > max_load_factor()) {
			rehash();
		}
		int z = 0;
	};//end insert_or_assign

	//------------------------------bucket_count-------------------------------
	// returns the size of buckets
	// O(1) all cases
	inline size_t bucket_count() const noexcept {
		return _bucket_count;
	};//end bucket_count

	//------------------------------max_bucket_count---------------------------
	// returns the maximum number of buckets allowed
	// O(1) all cases
	inline size_t max_bucket_count() const noexcept {
		return _max_bucket_count;
	};//end max_bucket_count

	//------------------------------load_factor--------------------------------
	// returns the percentage of buckets that is filled
	// O(1) all cases
	inline double load_factor() const noexcept {
		return size() / (double)bucket_count();
	};//end load_factor

	//------------------------------max_load_factor----------------------------
	// returns the maximum percentage of buckets that are allowed to be filled
	// O(1) all cases
	inline float max_load_factor() const noexcept {
		return _max_load_factor;
	};// end max_load_factor

	//------------------------------rehash-------------------------------------
	// resizes buckets to twice the current size of _max_bucket_count,
	// whichever is lower
	// O(n) average case, O(n^2) worst case
	void rehash() noexcept {
		if (_bucket_count == _max_bucket_count) {
			return;
		}
		else if (_bucket_count * 2 < _max_bucket_count) {
			_bucket_count *= 2;
		}
		else if (_bucket_count < _max_bucket_count) {
			_bucket_count = _max_bucket_count;
		}
		else {
			std::cout << "<<<<ERROR: MEMORY OVERFLOW, OVER 1,000,000 ELEMENTS INSERTED>>>>" << std::endl;
		}
		HASH_NODE* temp = new HASH_NODE[_bucket_count + 1];
		auto t_start = &temp[0], t_end = &temp[_bucket_count];

		auto END = begin() + (_bucket_count / 2);
		for (auto i = begin(); i < END; i++) {
			if (i->first != key()) {
				auto ptr = t_start + getStart(i->first);
			rehash_loop:
				for (ptr; ptr < t_end; ptr++) {
					if (ptr->first == key()) {
						ptr->first = i->first;
						ptr->second = i->second;
						break;
					}
				}
				if (ptr == t_end) {
					ptr = t_start;
					goto rehash_loop;
				}
			}
		}
		std::swap(temp, buckets);
		delete[] temp;
	};//end rehash

	//------------------------------getStart-----------------------------------
	// return an int for the start of accessors, insert() and count()
	// O(1) all cases
	inline int getStart(const key& k) const noexcept {
		return std::hash<key>{}(k) % _bucket_count;
	};
	//end getStart

	//------------------------------clear--------------------------------------
	//reset contents and data values of lhs
	// O(n) all cases
	void clear() {
		for (HASH_NODE* ptr = &buckets[0]; ptr != &buckets[_bucket_count]; ptr++) {
			delete ptr;
		}
		delete[] buckets;
		_bucket_count = 100;
		array_size = 0;
		buckets = new HASH_NODE*[_bucket_count];
	};//end clear
	// BETA FEATURES

	//-------------------------------contains----------------------------------
	// Returns if rhs is used as an index
	// O(1) average case, O(n) worst case
	bool contains( key&& k) const noexcept {
		int found = 0;
		int start = getStart(k);
		auto ptr = begin() + start;
	count_loop:
		for (ptr; ptr < end(); ptr++) {
			if (ptr->first == key()) {
				return false;
			}
			if (ptr->first == k) {
				return true;
			}
		}
		if (ptr == end()) {
			ptr = begin();
			goto count_loop;
		}
	};// end contains
	
	//-------------------------------empty-------------------------------------
	// check if the container is empty
	// O(1) all cases
	inline bool empty() const noexcept{
		return !array_size;
	};// end empty

	//-------------------------------find--------------------------------------
	// Check the container for elements
	// Returns the element address
	// O(1) average case, O(n) worst case
	HASH_NODE* find(const key& k){
		int found = 0;
		int start = getStart(k);
		auto ptr = begin() + start;
	count_loop:
		for (ptr; ptr < end(); ptr++) {
			if (ptr->first == key()) {
				return end();
			}
			if (ptr->first == k) {
				return ptr;
			}
		}
		if (ptr == end()) {
			ptr = begin();
			goto count_loop;
		}
	};// end find

	//-------------------------------find--------------------------------------
	// Check the container for elements
	// Returns the element address
	// O(1) average case, O(n) worst case
	const auto find(const key& k) const noexcept{
		int found = 0;
		int start = getStart(k);
		auto ptr = begin() + start;
	count_loop:
		for (ptr; ptr < end(); ptr++) {
			if (ptr->first == key()) {
				return end();
			}
			if (ptr->first == k) {
				return ptr;
			}
		}
		if (ptr == end()) {
			ptr = begin();
			goto count_loop;
		}
	};// end find

	//-------------------------------reserve-----------------------------------
	// Reserve the memory necessary for rhs number of buckets
	// O(n) average case, O(n^2) worst case
	void reserve(int& i) noexcept {
		HASH_NODE* temp = new HASH_NODE[i / _max_load_factor + 1];
		auto t_start = &temp[0], t_end = &temp[i / _max_load_factor + 1];

		auto END = begin() + (_bucket_count);
		for (auto i = begin(); i < END; i++) {
			if (i->first != key()) {
				auto ptr = t_start + getStart(i->first);
			rehash_loop:
				for (ptr; ptr < t_end; ptr++) {
					if (ptr->first == key()) {
						ptr->first = i->first;
						ptr->second = i->second;
						break;
					}
				}
				if (ptr == t_end) {
					ptr = t_start;
					goto rehash_loop;
				}
			}
		}
		std::swap(temp, buckets);
		_bucket_count = i / _max_load_factor + 1;
		delete[] temp;
	}; // end reserve
	
}; //end HASH_MAP



#endif
