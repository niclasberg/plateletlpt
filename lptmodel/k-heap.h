/***************************************************************************
 *   Copyright (C) 2010 by Leandro Graciá Gil                              *
 *   leandro.gracia.gil@gmail.com                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/**
 * \file k-heap.h
 * \brief Template for k-heaps holding the best k elements (logarithmic).
 * \author Leandro Graciá Gil
*/

#ifndef _K_HEAP_H_
#define _K_HEAP_H_

// Include indirect heaps.
#include "indirect_heap.h"

// Include STL less comparison template (used by default).
#include <functional>

/**
 * \brief Define a reference-based heap that holds the K best elements on it allowing to push them in logarithmic time.
 *
 * \note Even if asyntotically optimal it could be slower than \link k_vector k-vectors\endlink for small values of K due to extra operations and cache misses.
 *
 * \tparam T Type of the data stored in the k-heap.
 * \tparam Compare Type of the comparison object. The class must inherit from \c std::binary_function an define its \c bool (\c const \c &T, \c const \c &T) \c const operator.
 * \tparam idx Type used for encoding data vector indices. Defaults to unsigned int, but can be reduced to short or char for small K values.
*/
template <typename T, typename Compare = std::less<T>, typename idx = unsigned int>
class k_heap {
public:

  // Constructors and destructors.
  k_heap(unsigned int k); ///< K-heap constructor.
  k_heap(const k_heap &heap); ///< Copy constructor.
  ~k_heap(); ///< Default destructor.

  // Assignment operator.
  k_heap &operator =(const k_heap &heap); ///< Assignment operator.

  // Comparison operator.
  bool operator ==(const k_heap &heap) const; ///< Comparison operator.

  // Heap operations.
  bool push(const T &elem); ///< Push an element into the K-heap. Worst element will be replaced when heap is full. Cost: O(log K).
  void pop_best(); ///< Pop the best element from the heap. Can be used for direct sorting. Cost: O(log K).
  void pop_worst(); ///< Pop the worst element from the heap. Can be used for reverse sorting. Cost: O(log K).

  // Heap properties.
  bool full() const; ///< Check if the heap is full (it has already K elements). Cost: O(1).
  bool empty() const; ///< Check if the heap is empty. Cost: O(1).
  unsigned int size() const; ///< Return the number of elements in the heap. Cost: O(1).

  const T &best() const; ///< Retrieve the best element from the heap, or the first data object if empty. Cost: O(1).
  const T &worst() const; ///< Retrieve the worst element from the heap, or the first data object if empty. Cost: O(1).

  unsigned int get_K() const; ///< Get the maximum number of elements stored in the heap (also the maximum heap size). Cost: O(1).
  unsigned int count() const; ///< Get the number of elements currently in the heap. Cost: O(1).


  // STL-based names for heap operations and properties.
  bool push_front(const T &elem) { return push(elem); } ///< Same as \link k_heap::push push\endlink.
  bool push_back (const T &elem) { return push(elem); } ///< Same as \link k_heap::push push\endlink.

  void pop_front() { pop_worst(); } ///< Same as \link k_heap::pop_worst\endlink.
  void pop_back()  { pop_best(); } ///< Same as \link k_heap::pop_best\endlink.

  const T &front() const { return worst();  } ///< Same as \link k_heap::worst\endlink.
  const T &back()  const { return best(); } ///< Same as \link k_heap::best\endlink.


protected:
  unsigned int K; ///< Maximum number of best elements stored (also maximum heap size).
  T *data; ///< Array of stored elements (0-indexed).

  const Compare &compare; ///< Comparison object used by internal heaps.
  static const idx root = 1; ///< Root index of the heap.

  indirect_heap<T, Compare, idx> bestHeap; ///< Heap storing best data indices.
  indirect_heap<T, std::binary_negate<Compare>, idx> worstHeap; ///< Heap storing worst data indices.
};

// Template implementation.
#include "k-heap.cpp"

#endif
