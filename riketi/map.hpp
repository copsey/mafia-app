#ifndef RIKETI_MAP
#define RIKETI_MAP

#include <memory>
#include <vector>

#include "iterator.hpp"
#include "container.hpp"

/*
   Notes

   A map is defined to be an associative container which is similar in
   functionality to std::map. In particular, a map type X should be such that
   X::value_type is of the form std::pair<const K, I>, where K is X::key_type.

   An item of a map is defined to be the second element of such a pair. Thus the
   item type of X is the same as X::mapped_type.
*/

namespace rkt {
   namespace map {
      using namespace rkt::container; // a map is a container

      /// The key type associated with Map.
      template <class Map>
      using key_type = typename Map::key_type;

      /// The item type associated with Map.
      ///
      /// In Map itself, this is referred to as mapped_type.
      template <class Map>
      using item_type = typename Map::mapped_type;

      /// Copy all of the keys in map into a vector.
      ///
      /// The order of the keys is preserved.
      template <class Map>
      std::vector<key_type<Map>> keys(const Map & map) {
         std::vector<key_type<Map>> vec{};
         vec.reserve(map.size());
         for (auto & pair: map) vec.push_back(pair.first);
         return vec;
      }

      /// Copy all of the items in map into a vector.
      ///
      /// The order of the items is preserved.
      template <class Map>
      std::vector<item_type<Map>> items(const Map & map) {
         std::vector<item_type<Map>> vec{};
         vec.reserve(map.size());
         for (auto & pair: map) vec.push_back(pair.second);
         return vec;
      }
   }


   /// An iterator that traverses the keys of a Map.
   ///
   /// The reference to each key is const.
   template <class Map>
   struct key_iterator {
      using map_type = Map;
      using map_iterator = map::iterator_type<map_type>;
      using map_const_iterator = map::const_iterator_type<map_type>;

      using value_type = map::key_type<map_type>;
      using difference_type = iterator::difference_type<map_const_iterator>;
      using pointer = const value_type *;
      using reference = const value_type &;
      using iterator_category = iterator::category<map_const_iterator>;

      key_iterator() = default;

      key_iterator(map_iterator p)
       : _p{p} { }

      key_iterator(map_const_iterator p)
       : _p{p} { }

      /// A map_const_iterator pointing to the same entry in the underlying
      /// map as this.
      operator map_const_iterator () const {
         return _p;
      }

      bool operator== (const key_iterator & x) const {
         return _p == x._p;
      }

      bool operator!= (const key_iterator & x) const {
         return _p != x._p;
      }

      key_iterator & operator++ () {
         ++_p;
         return *this;
      }

      key_iterator operator++ (int) {
         return _p++;
      }

      key_iterator & operator-- () {
         --_p;
         return *this;
      }

      key_iterator operator-- (int) {
         return _p--;
      }

      reference operator* () const {
         return (*_p).first;
      }

      pointer operator-> () const {
         return std::addressof((*_p).first);
      }

   private:
      map_const_iterator _p{};
   };

   /// Make a key_iterator that points to the beginning of map.
   template <class Map>
   key_iterator<Map> key_begin(const Map & map) {
      return key_iterator<Map>{map.begin()};
   }

   /// Make a key_iterator that points to the end of map.
   template <class Map>
   key_iterator<Map> key_end(const Map & map) {
      return key_iterator<Map>{map.end()};
   }

   /// An iterator that traverses the items of a `Map`.
   template <class Map>
   struct item_iterator {
      using map_type = Map;
      using map_iterator = map::iterator_type<map_type>;

      using value_type = map::item_type<map_type>;
      using difference_type = iterator::difference_type<map_iterator>;
      using pointer = value_type *;
      using reference = value_type &;
      using iterator_category = iterator::category<map_iterator>;

      item_iterator() = default;

      item_iterator(map_iterator p)
       : _p{p} { }

      /// A map_iterator pointing to the same entry in the underlying map
      /// as this.
      operator map_iterator () const {
         return _p;
      }

      bool operator== (const item_iterator & x) const {
         return _p == x._p;
      }

      bool operator!= (const item_iterator & x) const {
         return _p != x._p;
      }

      item_iterator & operator++ () {
         ++_p;
         return *this;
      }

      item_iterator operator++ (int) {
         return _p++;
      }

      item_iterator & operator-- () {
         --_p;
         return *this;
      }

      item_iterator operator-- (int) {
         return _p--;
      }

      reference operator* () const {
         return (*_p).second;
      }

      pointer operator-> () const {
         return std::addressof((*_p).second);
      }

   private:
      map_iterator _p{};
   };

   /// An iterator that traverses the items of a Map.
   ///
   /// The reference to each item is const.
   template <class Map>
   struct const_item_iterator {
      using map_type = Map;
      using map_iterator = map::iterator_type<map_type>;
      using map_const_iterator = map::const_iterator_type<map_type>;

      using value_type = map::item_type<map_type>;
      using difference_type = iterator::difference_type<map_const_iterator>;
      using pointer = const value_type *;
      using reference = const value_type &;
      using iterator_category = iterator::category<map_const_iterator>;

      const_item_iterator() = default;

      const_item_iterator(const item_iterator<map_type> & x)
       : _p{static_cast<map_iterator>(x)} { }

      const_item_iterator(map_iterator p)
       : _p{p} { }

      const_item_iterator(map_const_iterator p)
       : _p{p} { }

      /// A map_const_iterator pointing to the same entry in the underlying
      /// map as this.
      operator map_const_iterator () const {
         return _p;
      }

      bool operator== (const const_item_iterator & x) const {
         return _p == x._p;
      }

      bool operator!= (const const_item_iterator & x) const {
         return _p != x._p;
      }

      const_item_iterator & operator++ () {
         ++_p;
         return *this;
      }

      const_item_iterator operator++ (int) {
         return _p++;
      }

      const_item_iterator & operator-- () {
         --_p;
         return *this;
      }

      const_item_iterator operator-- (int) {
         return _p--;
      }

      reference operator* () const {
         return (*_p).second;
      }

      pointer operator-> () const {
         return std::addressof((*_p).second);
      }

   private:
      map_const_iterator _p{};
   };

   /// Make an item_iterator that points to the beginning of map.
   template <class Map>
   item_iterator<Map> item_begin(Map & map) {
      return item_iterator<Map>{map.begin()};
   }

   /// Make a const_item_iterator that points to the beginning of map.
   template <class Map>
   const_item_iterator<Map> item_begin(const Map & map) {
      return const_item_iterator<Map>{map.begin()};
   }

   /// Make an item_iterator that points to the end of map.
   template <class Map>
   item_iterator<Map> item_end(Map & map) {
      return item_iterator<Map>{map.end()};
   }

   /// Make a const_item_iterator that points to the end of map.
   template <class Map>
   const_item_iterator<Map> item_end(const Map & map) {
      return const_item_iterator<Map>{map.end()};
   }
}

#endif
