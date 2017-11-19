#ifndef RIKETI_REF
#define RIKETI_REF

#include <memory>
#include <utility>

namespace rkt {
	// A struct encapsulating a reference to a value of type `t`.
	// 
	// Unlike normal references, the address stored by a `ref` can freely be 
	// changed. The only restriction is that a `ref` must always direct to a
	// non-null object.
	//
	// As much as possible, a `ref` is plain data: essentially it wraps a C
	// pointer to the underlying object. Arithmetic operators are not supported,
	// since `ref`s must always refer to a non-null object.
	//
	// No ownership of the object directed to by the stored reference is implied.
	template <typename t>
	struct ref {
		// The type of the value being referenced.
		using type = t;
		
		// Create a reference to `obj`.
		constexpr ref(type & obj) noexcept
			: ptr{std::addressof(obj)}
		{ }
		
		ref(type &&) = delete;
		
		// Check whether `this` and `r` refer to the same object.
		constexpr bool operator== (const ref & r) const noexcept {
			return ptr == r.ptr;
		}
		
		// Check whether `this` and `r` refer to different objects.
		constexpr bool operator!= (const ref & r) const noexcept {
			return ptr != r.ptr;
		}
		
		// Get the stored reference.
		constexpr type & operator* () const {
			return *ptr;
		}
		
		// Act on the object referred to by `this`.
		constexpr type * operator-> () const {
			return ptr;
		}
		
		// Replace the current reference with a reference to `obj`.
		void set(type & obj) noexcept {
			ptr = std::addressof(obj);
		}
		
		void set(type &&) = delete;
		
		// Ѕwap the stored references of `this` and `other`.
		void swap(ref & other) noexcept {
			std::swap(ptr, other.ptr);
		}
		
	private:
		type * ptr;
	};
	
	// Swap the stored references of `r1` and `r2`.
	template <typename t>
	void swap(ref<t> & r1, ref<t> & r2) noexcept {
		r1.swap(r2);
	}
}

#endif
