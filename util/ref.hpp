#ifndef MAF_UTIL_REF
#define MAF_UTIL_REF

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace maf::util {
	// A struct encapsulating a reference to a value of type `T`.
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
	template <typename T>
	struct ref {
		// The type of the value being referenced.
		using element_type = T;
		
		// Create a reference to `obj`.
		constexpr ref(element_type & obj) noexcept
			: ptr{std::addressof(obj)}
		{ }
		
		ref(element_type &&) = delete;
		
		// Convert `this` to a reference to the same element, but using the
		// base class `B`.
		//
		// This operator allows implicit conversion.
		template <typename B,
		          std::enable_if_t<std::is_base_of_v<B, element_type>, int> = 0>
		constexpr operator ref<B> () const {
			return ref<B>(static_cast<B&>(*ptr));
		}
		
		// Convert `this` to a reference to the same element, but using the
		// derived class `D`.
		//
		// This operator only allows explicit conversion.
		template <typename D,
		          std::enable_if_t<std::is_base_of_v<element_type, D>, int> = 0>
		explicit constexpr operator ref<D> () const {
			return ref<D>(static_cast<D&>(*ptr));
		}
		
		// Check whether `this` and `r` refer to the same object.
		constexpr bool operator== (const ref & r) const noexcept {
			return ptr == r.ptr;
		}
		
		// Check whether `this` and `r` refer to different objects.
		constexpr bool operator!= (const ref & r) const noexcept {
			return ptr != r.ptr;
		}
		
		// Get the stored reference.
		constexpr element_type & operator* () const {
			return *ptr;
		}
		
		// Act on the object referred to by `this`.
		constexpr element_type * operator-> () const {
			return ptr;
		}
		
		// Replace the current reference with a reference to `obj`.
		void set(element_type & obj) noexcept {
			ptr = std::addressof(obj);
		}
		
		void set(element_type &&) = delete;
		
		// Ѕwap the stored references of `this` and `other`.
		void swap(ref & other) noexcept {
			std::swap(ptr, other.ptr);
		}
		
	private:
		element_type * ptr;
		
		friend class std::hash<ref>;
	};
	
	// Swap the stored references of `r1` and `r2`.
	template <typename T>
	void swap(ref<T> & r1, ref<T> & r2) noexcept {
		r1.swap(r2);
	}
}

namespace std {
	// Function object for computing the hash of a `ref`.
	template <typename T>
	struct hash<maf::util::ref<T>> {
		size_t operator() (const maf::util::ref<T> & r) const {
			return hash<T*>()(r.ptr);
		}
	};
}

#endif
