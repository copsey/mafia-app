#ifndef RIKETI_REF
#define RIKETI_REF

#include <memory>
#include <utility>

namespace rkt {
   /// A struct encapsulating a reference to an object of type T.
   ///
   /// The reference is not necessarily fixed to a particular object.
   template <class T>
   struct ref {
      /// The type of the object being referred to.
      using type = T;

      /// Create a reference to t.
      ref (T & t)
       : p{std::addressof(t)} { }

      ref (T &&) = delete;

      /// Check whether this and r refer to the same object.
      bool operator== (const ref & r) const {
         return p == r.p;
      }

      /// Check whether this and r refer to different objects.
      bool operator!= (const ref & r) const {
         return p != r.p;
      }

      /// Get the stored reference.
      T & get () const {
         return *p;
      }

      /// Get the stored reference.
      operator T & () const {
         return *p;
      }

      /// Replace the current reference with a reference to t.
      void set (T & t) {
         p = std::addressof(t);
      }

      void set (T &&) = delete;

      /// Act on the object being referred to.
      T * operator-> () const {
         return p;
      }

      /// Swap the references stored by this and r.
      void swap (ref & r) {
         std::swap(p, r.p);
      }

   private:
      T * p;
   };

   /// Swap the references stored by r1 and r2.
   template <class T>
   void swap (ref<T> & r1, ref<T> & r2) {
      r1.swap(r2);
   }


   /// A struct encapsulating a reference to a fixed object of type T.
   ///
   /// The reference always refers to the same object.
   template <class T>
   struct fixed_ref {
      /// The type of the object being referred to.
      using type = T;

      /// Create a reference to t.
      fixed_ref (T & t)
       : p{std::addressof(t)} { }

      fixed_ref (T &&) = delete;

      fixed_ref (const fixed_ref &) = default;
      fixed_ref (fixed_ref &&) = default;
      ~fixed_ref () = default;
      // no assignment operators

      /// Check whether this and r refer to the same object.
      bool operator== (const fixed_ref & r) const {
         return p == r.p;
      }

      /// Check whether this and r refer to different objects.
      bool operator!= (const fixed_ref & r) const {
         return p != r.p;
      }

      /// Get the stored reference.
      T & get () const {
         return *p;
      }

      /// Get the stored reference.
      operator T & () const {
         return *p;
      }

      /// Act on the object being referred to.
      T * operator-> () const {
         return p;
      }

   private:
      T * p;
   };
}

#endif
