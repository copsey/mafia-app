#ifndef RIKETI_BOX
#define RIKETI_BOX

#include <exception>
#include <memory>
#include <type_traits>
#include <utility>

/* Declarations */

namespace rkt {
   // Similar to a unique_ptr, with static rather than dynamic memory.
   // Can be more efficient for small data types (e.g. int, double), but is
   // probably less efficient for large data types (e.g. char[1024]).
   template <class T>
   struct box {
      // Signifies that the contents of an empty box were accessed.
      struct empty_access: std::exception {
         const char * what() const noexcept override;
      };

      // The type of object being contained in the box.
      using type = T;

      // Creates an empty box.
      box() = default;
      // Creates a box containing t.
      box(const T &t);
      box(T &&t);
      // Creates a box from another box.
      box(const box &b) = default;
      box(box &&b);

      // Destroys the box and its contents.
      ~box() = default;

      // Assigns to this box the contents of another box.
      box & operator= (const box &b) = default;
      box & operator= (box &&b);

      // Checks whether anything is in the box.
      bool is_empty() const;
      bool is_full() const;

      // Checks whether two boxes are equal.
      // Equality holds iff both boxes are empty or both boxes contain equal
      // objects.
      bool operator== (const box &b) const;
      bool operator!= (const box &b) const;

      // Returns a reference to the object in the box.
      // Throws an exception if the box is empty.
            T & get();
      const T & get() const;
      // Allows direct access to the object inside the box.
      // The results of using these operations on an empty box are undefined.
            T & operator* ();
      const T & operator* () const;
            T * operator-> ();
      const T * operator-> () const;

      // Takes the object out of the box.
      // Throws an exception if the box is empty.
      T take();

      // Puts t into the box.
      void put(const T &t);
      void put(T &&t);

      // Removes the object from the box, if one exists.
      void clear();

      // Swaps the contents of this box with b.
      void swap(box &b);

   private:
      T _t{};
      bool _empty{true};

      void throw_if_empty() const;
   };

   // Swaps the contents of two boxes.
   template <class T>
   void swap(box<T> &b1, box<T> &b2);
}




/* Definitions */

template <class T>
const char * rkt::box<T>::empty_access::what() const noexcept {
   return "Attempted to access an object inside an empty rkt::box.";
}

template <class T>
rkt::box<T>::box(const T &t)
 : _t{t}, _empty{false} {

}

template <class T>
rkt::box<T>::box(T &&t)
 : _t{t}, _empty{false} {

}

template <class T>
rkt::box<T>::box(box &&b)
 : _t{std::move(b._t)}, _empty{b._empty} {
   b._empty = true;
}

template <class T>
rkt::box<T> & rkt::box<T>::operator= (box &&b) {
   _t = std::move(b._t);
   _empty = b._empty;
   b._empty = true;
   return *this;
}

template <class T>
bool rkt::box<T>::is_empty() const {
   return _empty;
}

template <class T>
bool rkt::box<T>::is_full() const {
   return !_empty;
}

template <class T>
bool rkt::box<T>::operator== (const box &b) const {
   return _empty ? b._empty : (b._empty ? false : (_t == b._t));
}

template <class T>
bool rkt::box<T>::operator!= (const box &b) const {
   return _empty ? !b._empty : (b._empty ? true : (_t != b._t));
}

template <class T>
T & rkt::box<T>::get() {
   throw_if_empty();
   return _t;
}

template <class T>
const T & rkt::box<T>::get() const {
   throw_if_empty();
   return _t;
}

template <class T>
T & rkt::box<T>::operator* () {
   return _t;
}

template <class T>
const T & rkt::box<T>::operator* () const {
   return _t;
}

template <class T>
T * rkt::box<T>::operator-> () {
   return std::addressof(_t);
}

template <class T>
const T * rkt::box<T>::operator-> () const {
   return std::addressof(_t);
}

template <class T>
T rkt::box<T>::take() {
   throw_if_empty();
   _empty = true;
   return std::move(_t);
}

template <class T>
void rkt::box<T>::put(const T &t) {
   _t = t;
   _empty = false;
}

template <class T>
void rkt::box<T>::put(T &&t) {
   _t = t;
   _empty = false;
}

template <class T>
void rkt::box<T>::clear() {
   if (std::is_trivially_destructible<T>::value) {
      _empty = true;
   } else {
      if (!_empty) {
         _t = T{};
         _empty = true;
      }
   }
}

template <class T>
void rkt::box<T>::swap(box &b) {
   using std::swap;
   swap(_t, b._t);
   swap(_empty, b._empty);
}

template <class T>
void rkt::box<T>::throw_if_empty() const {
   if (_empty) throw empty_access{};
}

template <class T>
void rkt::swap(box<T> &b1, box<T> &b2) {
   b1.swap(b2);
}

#endif
