#ifndef RIKETI_REF
#define RIKETI_REF

#include <memory>
#include <utility>

/* Declarations */

namespace rkt {
   // Encapsulates a reference to an object of type T.
   // The reference is not fixed, and can be changed to refer to a different
   // object.
   template <class T>
   struct ref {
      // The type of object referred to.
      using type = T;

      // Creates a reference to t.
      ref(T &t);
      ref(T &&) = delete;
      // Copies the reference stored in r.
      ref(const ref &r) = default;
      ref(ref &&) = delete;

      // Destroys the reference, leaving the referred-to object unchanged.
      ~ref() = default;

      // Copies the reference in r.
      ref & operator= (const ref &r) = default;
      ref & operator= (ref &&) = delete;

      // Checks whether this reference and r are equal.
      // Equality holds iff both references refer to the same object.
      bool operator== (const ref &r) const;
      bool operator!= (const ref &r) const;

      // Gets the reference being stored.
      operator T & () const;
      T & get() const;

      // Stores a reference to t.
      void set(T &t);
      void set(T &&) = delete;

      // Swaps the references stored by this and r.
      void swap(ref &r);

   private:
      T *_p;
   };

   // Swaps the references stored by r1 and r2.
   template <class T>
   void swap(ref<T> &r1, ref<T> &r2);


   // Encapsulates a reference to an object of type T.
   // The reference is fixed, and will always refer to the same object.
   template <class T>
   struct fixed_ref {
      // The type of object referred to.
      using type = T;

      // Creates a reference to t.
      fixed_ref(T &t);
      fixed_ref(T &&) = delete;
      // Copies the reference stored in r.
      fixed_ref(const fixed_ref &r) = default;
      fixed_ref(fixed_ref &&) = delete;

      // Destroys the reference, leaving the referred-to object unchanged.
      ~fixed_ref() = default;

      // Checks whether this reference and r are equal.
      // Equality holds iff both references refer to the same object.
      bool operator== (const fixed_ref &r) const;
      bool operator!= (const fixed_ref &r) const;

      // Gets the reference being stored.
      operator T & () const;
      T & get() const;

   private:
      T *_p;
   };
}




/* Definitions */

template <class T>
rkt::ref<T>::ref(T &t)
 : _p{std::addressof(t)} {

}

template <class T>
bool rkt::ref<T>::operator== (const ref &r) const {
   return _p == r._p;
}

template <class T>
bool rkt::ref<T>::operator!= (const ref &r) const {
   return _p != r._p;
}

template <class T>
rkt::ref<T>::operator T & () const {
   return *_p;
}

template <class T>
T & rkt::ref<T>::get() const {
   return *_p;
}

template <class T>
void rkt::ref<T>::set(T &t) {
   _p = std::addressof(t);
}

template <class T>
void rkt::ref<T>::swap(ref &r) {
   std::swap(_p, r._p);
}

template <class T>
void rkt::swap(ref<T> &r1, ref<T> &r2) {
   r1.swap(r2);
}

template <class T>
rkt::fixed_ref<T>::fixed_ref(T &t)
 : _p{std::addressof(t)} {

}

template <class T>
bool rkt::fixed_ref<T>::operator== (const fixed_ref &r) const {
   return _p == r._p;
}

template <class T>
bool rkt::fixed_ref<T>::operator!= (const fixed_ref &r) const {
   return _p != r._p;
}

template <class T>
rkt::fixed_ref<T>::operator T & () const {
   return *_p;
}

template <class T>
T & rkt::fixed_ref<T>::get() const {
   return *_p;
}

#endif
