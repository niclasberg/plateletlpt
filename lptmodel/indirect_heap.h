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
 * \file indirect_heap.h
 * \brief Template for index-based indirect heaps.
 * \author Leandro Graciá Gil
*/

#ifndef _INDIRECT_HEAP_H_
#define _INDIRECT_HEAP_H_

// Include 'less' comparison template (used by default).
#include <functional>

/**
 * \brief Define a reference-based heap over an array of existing data that
 *        can be accessed and modified while mantaining the heap properties.
 *
 * Element indices can be pushed up to a maximum size.
 *
 * \tparam T Type of the data stored in the k-vector.
 * \tparam Compare Type of the comparison object. Defaults to std::less<T>.
 * \tparam idx Type used for encoding data vector indices. Defaults to unsigned int, but can be reduced to short or char for small K values.
*/
template <typename T, typename Compare = std::less<T>, typename idx = unsigned int>
class indirect_heap {
protected:

  mutable const T *data; ///< Array of stored elements. Shifted to be 1-indexed.
  idx *heap; ///< Heap structure built as indices to data.
  idx *inverse; ///< Inverse heap position indices.

  unsigned int size; ///< Heap maximum size.
  unsigned int used; ///< Number of elements used in the heap.
  unsigned int last; ///< Position in the data array where last element is (1-indexed).

  const Compare &compare; ///< Comparison object.
  static const idx root = 1; ///< Root index of the heap.

  inline idx parent_idx(idx index) { return index >> 1; } ///< Get the index of the parent of a node.
  inline idx left_idx (idx index) { return index << 1; } ///< Get the index of the left child of a node.
  inline idx right_idx(idx index) { return (index << 1) + 1; } ///< Get the index of the right child of a node.

  inline void swap_elements(idx i1, idx i2); ///< Swap indices and references from a pair of elements.
  inline void heapify_element(idx index); ///< Adjust the heap structure after inserting a new element in \a index.
  bool heapify_upwards(idx index); ///< Adjust the heap structure upwards after inserting a new element in \a index.
  void heapify_downwards(idx index); ///< Adjust the heap structure downwards after inserting a new element in \a index.

public:

  // Constructors and destructors.
  indirect_heap(const T *data, unsigned int size, unsigned int maxSize = 0, const Compare &c = Compare());
  indirect_heap(const indirect_heap &heap);
  ~indirect_heap();

  // Assignment operator (only the heap structure is copied, not the data pointer).
  indirect_heap &operator = (const indirect_heap &heap);

  // Comparison operator (only heap structure is compared, not data pointers or data itself).
  bool operator == (const indirect_heap &heap) const;

  // Data pointer adjustment.
  void setData(const T *data); ///< Update the pointer to data. Same size and structure is assumed. May require to rebuild the heap.

  // Usual heap operations.
  bool push(idx index); ///< Push an element into the heap. Cost: O(log n).
  T &pop(); ///< Pop the top element from the heap. Cost: O(log n).

  // Extended heap operations.
  bool inHeap(idx index) const; ///< Check if an object is the heap. Cost: O(1).
  bool remove(idx index); ///< Remove an object from the heap. Cost: O(log n).
  bool update(idx index); ///< Update the heap with the new value of an object. Cost: O(log n).
  void updateAll(); ///< Build the heap stucture again. Cost: O(n).
  bool swap(idx index1, idx index2); ///< Update the heap to reflect the swap of two elements in the data array. Cost: O(1).

  // Subscript operators.
  const T & operator [](idx index) const;
  T & operator [](idx index);

  // Heap properties.
  unsigned int maxSize() const;
  unsigned int dataSize() const;
  unsigned int count() const;
  bool empty() const;
  idx topIndex() const;
  T &top() const;
};

// Template implementation.
#include "indirect_heap.cpp"

#endif
