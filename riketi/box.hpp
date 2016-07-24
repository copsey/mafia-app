#ifndef RIKETI_BOX
#define RIKETI_BOX

#include <exception>
#include <memory>
#include <type_traits>
#include <utility>

namespace rkt {
   /// A container which is either empty or contains exactly one element of
   /// type T.
   template <class T>
   struct box {
      /// An exception signifying that the contents of an empty box were
      /// accessed.
      struct empty_access: std::exception {
         const char * what() const noexcept override {
            return "rkt::box: attempt to access object in empty box";
         }
      };

      /// The type of object contained in the box.
      using type = T;

      /// Create an empty box.
      box() = default;

      /// Create a box containing t.
      box(T && t): _t{t}, _empty{false} { }

      /// Create a box containing a copy of t.
      box(const T & t): _t{t}, _empty{false} { }

      box(const box &) = default;

      box(box && b): _t{std::move(b._t)}, _empty{b._empty} {
         b._empty = true;
      }

      ~box() = default;

      box & operator= (const box &) = default;

      box & operator= (box && b) {
         _t = std::move(b._t);
         _empty = b._empty;
         b._empty = true;
         return *this;
      }

      /// Check whether the box is empty.
      bool is_empty() const {
         return _empty;
      }

      /// Check whether the box is full.
      bool is_full() const {
         return !_empty;
      }

      /// Check whether this and b contain equal objects, or are both empty.
      bool operator== (const box &b) const {
         return _empty ? b._empty : (b._empty ? false : (_t == b._t));
      }

      /// Check whether this and b contain unequal objects, or one is empty
      /// and the other is full.
      bool operator!= (const box &b) const {
         return _empty ? !b._empty : (b._empty ? true : (_t != b._t));
      }

      /// Obtain a reference to the object in the box.
      ///
      /// If the box is empty, an empty_access is thrown.
      T & get() {
         throw_if_empty();
         return _t;
      }

      /// Obtain a const reference to the object in the box.
      ///
      /// If the box is empty, an empty_access is thrown.
      const T & get() const {
         throw_if_empty();
         return _t;
      }

      /// Allows direct access to the object inside the box.
      ///
      /// The result of using this operation on an empty box is undefined.
      T & operator* () {
         return _t;
      }

      /// Allows direct access to the object inside the box.
      ///
      /// The result of using this operation on an empty box is undefined.
      const T & operator* () const {
         return _t;
      }

      /// Allows direct access to the object inside the box.
      ///
      /// The result of using this operation on an empty box is undefined.
      T * operator-> () {
         return std::addressof(_t);
      }

      /// Allows direct access to the object inside the box.
      ///
      /// The result of using this operation on an empty box is undefined.
      const T * operator-> () const {
         return std::addressof(_t);
      }

      /// Take the object out of the box.
      ///
      /// If the box is empty, an empty_access is thrown.
      T take() {
         throw_if_empty();
         _empty = true;
         return std::move(_t);
      }

      /// Put t into the box.
      void put(T && t) {
         _t = t;
         _empty = false;
      }

      /// Put a copy of t into the box.
      void put(const T & t) {
         _t = t;
         _empty = false;
      }

      /// Remove the object from the box, if one exists.
      void clear() {
         if (std::is_trivially_destructible<T>::value) {
            _empty = true;
         } else {
            if (!_empty) {
               _t = T{};
               _empty = true;
            }
         }
      }

      /// Swap the contents of this and b.
      void swap(box & b) {
         using std::swap;
         swap(_t, b._t);
         swap(_empty, b._empty);
      }

   private:
      T _t{};
      bool _empty{true};

      void throw_if_empty() const {
         if (_empty) throw empty_access{};
      }
   };

   /// Swap the contents of b1 and b2.
   template <class T>
   void swap(box<T> & b1, box<T> & b2) {
      b1.swap(b2);
   }
}

#endif
