#ifndef RIKETI_BOX
#define RIKETI_BOX

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace rkt {
	// An exception signifying that an attempt was made to access the contents
	// of an empty `box`.
	struct empty_box_access: std::logic_error {
		using std::logic_error::logic_error;
	};
	
	
	// A container which is either empty or contains exactly one element of
	// type `T`.
	//
	// `T` must be:
	//   + default constructible,
	//   + copy constructible & assignable,
	//   + move constructible & assignable.
	template <typename T>
	struct [[deprecated("use std::optional instead")]] box
	{
		// The type of object contained in the box.
		using type = T;
		
		// Create an empty box.
		box() = default;
		
		// Create a box containing `t`.
		box(T && t): _t{t}, _empty{false} { }
		
		// Create a box containing a copy of `t`.
		box(const T & t): _t{t}, _empty{false} { }
		
		// Create a box whose contents are copied from `b`.
		box(const box & b) = default;
		
		// Create a box whose contents are taken from `b`.
		box(box && b):
			_t{std::move(b._t)}, _empty{b._empty}
		{
			b._empty = true;
		}
		
		// Destroy `this` and its contents.
		~box() = default;
		
		// Copy the contents of `b` into `this`.
		box & operator= (const box & b) = default;
		
		// Move the contents of `b` into `this`.
		box & operator= (box && b) {
			_t = std::move(b._t);
			_empty = b._empty;
			
			b._empty = true;
			
			return *this;
		}
		
		// Check if either:
		// - `this` and `b` are both empty, or
		// - `this` and `b` are both non-empty, and `this->get() == b.get()`.
		bool operator== (const box & b) const {
			return _empty ? b._empty : (b._empty ? false : (_t == b._t));
		}
		
		// Equivalent to `!(*this == b)`.
		bool operator!= (const box & b) const {
			return _empty ? !b._empty : (b._empty ? true : (_t != b._t));
		}
		
		// Allows direct access to the object inside `this`.
		//
		// pre:
		// - `this` is not empty.
		T & operator* () {
			return _t;
		}
		
		// Allows direct access to the object inside `this`.
		//
		// pre:
		// - `this` is not empty.
		const T & operator* () const {
			return _t;
		}
		
		// Allows direct access to the object inside `this`.
		//
		// pre:
		// - `this` is not empty.
		T * operator-> () {
			return std::addressof(_t);
		}
		
		// Allows direct access to the object inside `this`.
		//
		// pre:
		// - `this` is not empty.
		const T * operator-> () const {
			return std::addressof(_t);
		}
		
		// Check whether `this` is empty.
		bool is_empty() const {
			return _empty;
		}
		
		// Check whether `this` contains an object.
		bool is_full() const {
			return !_empty;
		}
		
		// Obtain a reference to the object in `this`.
		//
		// throws:
		// - `empty_box_access` if `this` is empty.
		T & get() {
			if (_empty) {
				throw empty_box_access{"rkt: cannot access object in an empty box"};
			}
			
			return _t;
		}
		
		// Obtain a const reference to the object in `this`.
		//
		// throws:
		// - `empty_box_access` if `this` is empty.
		const T & get() const {
			if (_empty) {
				throw empty_box_access{"rkt: cannot access object in an empty box"};
			}
			
			return _t;
		}
		
		// Take the object out of `this`.
		//
		// throws:
		// - `empty_box_access` if `this` is empty.
		T take() {
			if (_empty) {
				throw empty_box_access{"rkt: cannot take object from an empty box"};
			}
			
			_empty = true;
			return std::move(_t);
		}
		
		// Put `t` into `this`.
		void put(T && t) {
			_t = t;
			_empty = false;
		}
		
		// Put a copy of `t` into `this`.
		void put(const T & t) {
			_t = t;
			_empty = false;
		}
		
		// Remove the object from `this`, if one exists.
		void clear() {
			if constexpr(std::is_trivially_destructible<T>::value) {
				_empty = true;
			} else {
				if (!_empty) {
					_t = T{};
					_empty = true;
				}
			}
		}
		
		// Swap the contents of `this` and `b`.
		void swap(box & b) {
			using std::swap;
			swap(_t, b._t);
			swap(_empty, b._empty);
		}
		
	private:
		T _t{};
		bool _empty{true};
	};
	
	// Swap the contents of `b1` and `b2`.
	template <typename T>
	void swap(box<T> & b1, box<T> & b2) {
		b1.swap(b2);
	}
}

#endif
