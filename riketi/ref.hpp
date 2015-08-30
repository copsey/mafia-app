#ifndef RIKETI_REF
#define RIKETI_REF

#include <memory>

namespace rkt {
   // Encapsulates a reference to an object of type T.
   // The reference is not fixed, and can be altered to refer to another object
   // at any time.
   template <class T>
   struct ref {
      using type = T;

      ref(T &t): _p{std::addressof(t)} { }
      ref(T &&) = delete;
      ref(const ref &) = default;
      ref(ref &&) = delete;

      ~ref() = default;

      ref & operator= (const ref &) = default;
      ref & operator= (ref &&) = delete;

      bool operator== (const ref &r) const { return _p == r._p; }
      bool operator!= (const ref &r) const { return _p != r._p; }

      operator T & () const { return *_p; }
      T & get() const { return *_p; }

      void set(T &t) { _p = std::addressof(t); }
      void set(T &&) = delete;

      void swap(ref &r) { std::swap(_p, r._p); }

   private:
      T *_p;
   };

   template <class T>
   void swap(ref<T> &r1, ref<T> &r2) {
      r1.swap(r2);
   }

   template <class T>
   ref<T> make_ref(T &t) {
      ref<T> r{t};
      return r;
   }

   template <class T>
   ref<T> make_ref(T &&) = delete;

   template <class T>
   ref<const T> make_cref(const T &t) {
      ref<const T> r{t};
      return r;
   }

   template <class T>
   ref<const T> make_cref(const T &&) = delete;


   // Encapsulates a reference to an object of type T.
   // The reference is fixed, and will always refer to the same object.
   template <class T>
   struct fixed_ref {
      using type = T;

      fixed_ref(T &t): _p{std::addressof(t)} { }
      fixed_ref(T &&) = delete;
      fixed_ref(const fixed_ref &) = default;
      fixed_ref(fixed_ref &&) = delete;

      ~fixed_ref() = default;

      fixed_ref & operator= (const fixed_ref &) = delete;
      fixed_ref & operator= (fixed_ref &&) = delete;

      bool operator== (const fixed_ref &fr) const { return _p == fr._p; }
      bool operator!= (const fixed_ref &fr) const { return _p != fr._p; }

      operator T & () const { return *_p; }
      T & get() const { return *_p; }

   private:
      T *_p;
   };

   template <class T>
   fixed_ref<T> make_fixed_ref(T &t) {
      fixed_ref<T> fr{t};
      return fr;
   }

   template <class T>
   fixed_ref<T> make_fixed_ref(T &&) = delete;

   template <class T>
   fixed_ref<const T> make_fixed_cref(const T &t) {
      fixed_ref<const T> fr{t};
      return fr;
   }

   template <class T>
   fixed_ref<const T> make_fixed_cref(const T &&) = delete;
}

#endif
