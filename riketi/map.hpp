#ifndef RIKETI_MAP
#define RIKETI_MAP

#include <map>
#include <vector>

namespace rkt {
   namespace map {
      // Collect all of the keys in a map into a vector.
      // The keys in the resulting vector are ordered the same as in the map.
      template <class Key, class Item, class Comp, class Alloc>
      std::vector<Key> keys(const std::map<Key, Item, Comp, Alloc> &map) {
         std::vector<Key> vec{};
         vec.reserve(map.size());
         for (const auto &pair : map) vec.push_back(pair.first);
         return vec;
      }

      // Collect all of the items in a map into a vector.
      // The items in the resulting vector are ordered the same as the
      //  corresponding keys in the map.
      template <class Key, class Item, class Comp, class Alloc>
      std::vector<Item> items(const std::map<Key, Item, Comp, Alloc> &map) {
         std::vector<Item> vec{};
         vec.reserve(map.size());
         for (const auto &pair : map) vec.push_back(pair.second);
         return vec;
      }

      // An iterator that traverses the keys of a map.
      // The reference to each key is const.
      template <class Key,
                class Item,
                class Comp = std::less<Key>,
                class Alloc = std::allocator<std::pair<const Key, Item>>>
      struct key_iterator {
         using map_type = std::map<Key, Item, Comp, Alloc>;
         using map_iterator = typename map_type::iterator;
         using map_const_iterator = typename map_type::const_iterator;

         using value_type = Key;
         using difference_type = typename map_const_iterator::difference_type;
         using pointer = const Key *;
         using reference = const Key &;
         using iterator_category = std::bidirectional_iterator_tag;

         key_iterator() : _p{} { }
         key_iterator(map_iterator p) : _p{p} { }
         key_iterator(map_const_iterator p) : _p{p} { }

         bool operator==(const key_iterator &iter) const {
            return _p == iter._p;
         }
         bool operator!=(const key_iterator &iter) const {
            return _p != iter._p;
         }

         key_iterator & operator++() { ++_p; return *this; }
         key_iterator operator++(int) { return _p++; }
         key_iterator & operator--() { --_p; return *this; }
         key_iterator operator--(int) { return _p--; }

         reference operator*() const { return (*_p).first; }
         pointer operator->() const { return &(*_p).first; }

      private:
         map_const_iterator _p;
      };



      template <class Key, class Item, class Comp, class Alloc>
      class item_const_iterator;

      // An iterator that traverses the items of a map.
      template <class Key,
                class Item,
                class Comp = std::less<Key>,
                class Alloc = std::allocator<std::pair<const Key, Item>>>
      struct item_iterator {
         friend class item_const_iterator<Key, Item, Comp, Alloc>;

         using map_type = std::map<Key, Item, Comp, Alloc>;
         using map_iterator = typename map_type::iterator;

         using value_type = Item;
         using difference_type = typename map_iterator::difference_type;
         using pointer = Item *;
         using reference = Item &;
         using iterator_category = std::bidirectional_iterator_tag;

         item_iterator() : _p{} { }
         item_iterator(map_iterator p) : _p{p} { }

         bool operator==(const item_iterator &iter) const {
            return _p == iter._p;
         }
         bool operator!=(const item_iterator &iter) const {
            return _p != iter._p;
         }

         item_iterator & operator++() { ++_p; return *this; }
         item_iterator operator++(int) { return _p++; }
         item_iterator & operator--() { --_p; return *this; }
         item_iterator operator--(int) { return _p--; }

         reference operator*() const { return (*_p).second; }
         pointer operator->() const { return &(*_p).second; }

      private:
         map_iterator _p;
      };



      // An iterator that traverses the items of a map.
      // The reference to each item is const.
      template <class Key,
                class Item,
                class Comp = std::less<Key>,
                class Alloc = std::allocator<std::pair<const Key, Item>>>
      struct item_const_iterator {
         using map_type = std::map<Key, Item, Comp, Alloc>;
         using map_iterator = typename map_type::iterator;
         using map_const_iterator = typename map_type::const_iterator;

         using value_type = Item;
         using difference_type = typename map_const_iterator::difference_type;
         using pointer = const Item *;
         using reference = const Item &;
         using iterator_category = std::bidirectional_iterator_tag;

         item_const_iterator() : _p{} { }
         item_const_iterator(const item_iterator<Key, Item, Comp, Alloc> &iter)
            : _p{iter._p} { }
         item_const_iterator(map_iterator p) : _p{p} { }
         item_const_iterator(map_const_iterator p) : _p{p} { }

         bool operator==(const item_const_iterator &iter) const {
            return _p == iter._p;
         }
         bool operator!=(const item_const_iterator &iter) const {
            return _p != iter._p;
         }

         item_const_iterator & operator++() { ++_p; return *this; }
         item_const_iterator operator++(int) { return _p++; }
         item_const_iterator & operator--() { --_p; return *this; }
         item_const_iterator operator--(int) { return _p--; }

         reference operator*() const { return (*_p).second; }
         pointer operator->() const { return &(*_p).second; }

      private:
         map_const_iterator _p;
      };
   }
}

#endif
