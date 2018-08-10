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
 * \file indirect_heap.cpp
 * \brief Template implementations for index-based indirect heaps.
 * \author Leandro Graciá Gil
*/

#include <algorithm>
#include <cassert>
#include <cstring>

/**
 * Build a mutable reference heap over an existing array.
 *
 * \param data Array with data.
 * \param size Number of elements in \a data.
 * \param maxSize Maximum heap size (should fit in \a data).
 * \param c Reference to a comparison object used to define an order.
*/
template <typename T, typename C, typename idx>
indirect_heap<T, C, idx>::indirect_heap(const T *data, unsigned int size, unsigned int maxSize, const C &c)
  : compare(c) {

  // Check for input data.
  if (data == NULL) {
    this->size = 0;
    this->used = 0;
    this->last = 0;
    this->heap = NULL;
    this->inverse = NULL;
    return;
  }

  // Initialize size and used counters.
  this->size = std::max<unsigned int>(size, maxSize);
  this->used = size;
  this->last = this->used;

  // Allocate auxiliar arrays.
  assert(heap = new idx[this->size]);
  assert(inverse = new idx[this->size]);

  // Shift arrays to make them 1-indexed.
  --heap;
  --inverse;

  // Set data array (shifted).
  this->data = data - 1;

  // Initialize indices.
  for (unsigned int i=1; i<=this->size; ++i)
    inverse[i] = heap[i] = i;

  // Build heap.
  for (unsigned int i=(used >> 1); i>=root; --i)
    heapify_downwards(i);
}

/** Default destructor */
template <typename T, typename C, typename idx>
indirect_heap<T, C, idx>::~indirect_heap() {

  // Release auxiliar arrays.
  delete []++heap;
  delete []++inverse;
}

/** Copy constructor. Heap structure and data pointer are copied from the input object. */
template <typename T, typename C, typename idx>
indirect_heap<T, C, idx>::indirect_heap(const indirect_heap &heap)
  : heap(NULL),
    inverse(NULL),
    size(0),
    compare(C(heap.compare)) {

  // Use asignment operator internally.
  *this = heap;

  // Use the data pointer of the input object.
  data = heap.data;
}

/** Assignment operator. Only heap structure is copied, not the data pointer. */
template <typename T, typename C, typename idx>
indirect_heap<T, C, idx> &indirect_heap<T, C, idx>::operator = (const indirect_heap &heap) {

  // Check self assignment.
  if (this == &heap)
    return *this;

  // Reallocate any previous arrays if sizes are different.
  if (size != heap.size) {

    // Set size.
    size = heap.size;

    // Reallocate arrays.
    if (this->heap)
      delete [](++this->heap);
    assert(this->heap = new idx[size]);
    --this->heap;

    if (inverse)
      delete [](++inverse);
    assert(inverse = new idx[size]);
    --inverse;
  }

  // Set heap attributes.
  used = heap.used;
  last = heap.last;

  // Copy array contents.
  memcpy(this->heap + 1, heap.heap + 1, size * sizeof(idx));
  memcpy(inverse + 1, heap.inverse + 1, size * sizeof(idx));

  // Return a reference to itself.
  return *this;
}

/** Comparison operator */
template <typename T, typename C, typename idx>
bool indirect_heap<T, C, idx>::operator == (const indirect_heap &heap) const {

  // Compare sizes, number of used elements.
  if (size != heap.size || used != heap.used)
    return false;

  // Compare heap and inverse index arrays.
  if (memcmp(this->heap + 1, heap.heap + 1, size * sizeof(idx)) != 0)
    return false;
  if (memcmp(inverse + 1, heap.inverse + 1, size * sizeof(idx)) != 0)
    return false;

  return true;
}

/**
 * Swap element indices both in the heap and the inverse permutation arrays.
 *
 * \param i1 Index of the first element to swap (1-indexed).
 * \param i2 Index of the second element to swap (1-indexed).
*/
template <typename T, typename C, typename idx>
void indirect_heap<T, C, idx>::swap_elements(idx i1, idx i2) {

  // Swap inverse indices.
  std::swap<idx>(inverse[heap[i1]], inverse[heap[i2]]);

  // Swap heap indices.
  std::swap<idx>(heap[i1], heap[i2]);
}

/**
 * Adjust an element position in the heap.
 *
 * \param index Index in the heap of the element to move (1-indexed).
*/
template <typename T, typename C, typename idx>
void indirect_heap<T, C, idx>::heapify_element(idx index) {

  // Try upwards first (less comparisons) and if not modified (comparison pruning), try downwards.
  if (!heapify_upwards(index))
    heapify_downwards(index);
}

/**
 * Move an element upwards the heap.
 *
 * \param index Index in the heap of the element to move (1-indexed).
 * \return \c true if heap was modified, \c false otherwise.
*/
template <typename T, typename C, typename idx>
bool indirect_heap<T, C, idx>::heapify_upwards(idx index) {

  // Keep swapping until heap condition is true or root node is reached.
  bool modified = false;
  while (index > root) {

    // Get parent index.
    idx parent = parent_idx(index);

    // Break if heap condition already true.
    if (compare(data[heap[parent]], data[heap[index]]))
      break;

    // Swap elements in heap.
    modified = true;
    swap_elements(index, parent);

    // Move current index to its parent.
    index = parent;
  }

  return modified;
}

/**
 * Move an element downwards the heap.
 *
 * \param index Index in the heap of the element to move (1-indexed).
*/
template <typename T, typename C, typename idx>
void indirect_heap<T, C, idx>::heapify_downwards(idx index) {

  // Keep swapping until heap condition is true or a leaf node is reached.
  while (index <= used) {

    // Look for maximum element in parent and child nodes.
    idx maxIndex = index;
    idx left = left_idx(index);
    if (left <= used) {
      maxIndex = compare(data[heap[left]], data[heap[index]]) ? left : maxIndex;
      idx right = right_idx(index);
      if (right <= used)
        maxIndex = compare(data[heap[right]], data[heap[maxIndex]]) ? right : maxIndex;
    }

    // Break if heap condition already true.
    if (maxIndex == index)
      break;

    // Swap elements.
    swap_elements(index, maxIndex);

    // Move to child node.
    index = maxIndex;
  }
}

/**
 * Update the pointer to real data. Same size and structure is assumed. Rebuilding the heap may be required after this.
 * \c NULL pointers are ignored.
 *
 * \param data New pointer to data.
*/
template <typename T, typename C, typename idx>
void indirect_heap<T, C, idx>::setData(const T *data) {

  // Shift data to make it 1-indexed.
  if (data)
    this->data = data - 1;
}

/**
 * Insert a new element in the heap, if it fits. Since data is never modified by the heap,
 * only existing data can be pushed into it, referenced by its index.
 *
 * \warning Only one copy of the same object can be in the heap at the same time.
 *
 * \param index Data array index of the element to push into the heap (0-indexed).
 * \return \c true if successful, \c false if heap is full or the element is already on the heap.
*/
template <typename T, typename C, typename idx>
bool indirect_heap<T, C, idx>::push(idx index) {

  // Make index 1-indexed.
  ++index;

  // Check if index is in the array boundaries.
  if (index > size || size < root)
    return false;

  // Get the index of the requested element in the heap.
  idx heap_index = inverse[index];

  // Check if item is already in the heap.
  if (heap_index <= used)
    return false;

  // Increase number of elements in use.
  ++used;

  // Move requested item into the new heap boundaries.
  if (used != heap_index)
    swap_elements(used, heap_index);

  // Move to its position in the heap.
  heapify_upwards(used);

  return true;
}

/**
 * Extract topmost element from the heap.
 *
 * \return Topmost element being extracted, or first element if heap is already \link indirect_heap::empty empty\endlink.
*/
template <typename T, typename C, typename idx>
T &indirect_heap<T, C, idx>::pop() {

  // Check size.
  if (empty())
    return const_cast<T *>(data)[root];

  // Get a reference to the current top.
  T &topmost = const_cast<T *>(data)[heap[root]];

  // Swap top and last elements.
  if (root != used)
    swap_elements(root, used);

  // Decrease the number of elements in the heap.
  --used;

  // Adjust heap downwards from the root.
  heapify_downwards(root);

  // Return stored reference.
  return topmost;
}

/**
 * Check if an element of the original array is still in the heap.
 *
 * \param index Index of the element to check (0-indexed).
 * \return \c true if still in the heap, \c false otherwise.
*/
template <typename T, typename C, typename idx>
bool indirect_heap<T, C, idx>::inHeap(idx index) const {

  // Shift the index to make it 1-indexed.
  ++index;

  // Check boundaries and heap data.
  return index >= root && index <= size && inverse[index] <= used;
}

/**
 * Access an element in the original array by its index. In case of modifying it,
 * \link indirect_heap::update update\endlink should be called afterwards.
 *
 * \param index Index of the element to get (0-indexed).
 * \return A reference to the requested element, or to the last one in case of out-of-bounds request.
*/
template <typename T, typename C, typename idx>
T &indirect_heap<T, C, idx>::operator [] (idx index) {

  // Shift the index to make it 1-indexed.
  ++index;

  // Get the corresponding element.
  if (index > size)
    index = size;
  return const_cast<T *>(data)[index];
}

/**
 * Access an element in the original array by its index.
 *
 * \param index Index of the element to get (0-indexed).
 * \return A non-mutable reference to the requested element, or to the last one in case of out-of-bounds request.
*/
template <typename T, typename C, typename idx>
const T &indirect_heap<T, C, idx>::operator [] (idx index) const {

  // Shift the index to make it 1-indexed.
  ++index;

  // Get the corresponding element.
  if (index > size)
    index = size;
  return data[index];
}

/**
 * Remove the specified element from the heap.
 *
 * \param index Index of the element in data array being removed from the heap (0-indexed).
 * \return \c true if successful, \c false if \a index is not in array.
*/
template <typename T, typename C, typename idx>
bool indirect_heap<T, C, idx>::remove(idx index) {

  // Check if the given index is inside the array (and shift it to make it 1-indexed).
  if (!inHeap(index++))
    return false;

  // Swap last and requested elements.
  idx heap_index = inverse[index];
  if (heap_index != used)
    swap_elements(heap_index, used);

  // Decrease number of used elements.
  --used;

  // Try to heapify in both directions (last element could come from a completely different branch).
  heapify_element(heap_index);

  return true;
}

/**
 * Update the heap structure after a item has been modified. Cost: O(log n)
 *
 * \param index Index of the item in the array that was modified (0-indexed).
 * \return \c true if successful, \c false if \a index is not in array.
*/
template <typename T, typename C, typename idx>
bool indirect_heap<T, C, idx>::update(idx index) {

  // Check if the given index is inside the array (and shift it to make it 1-indexed).
  if (!inHeap(index++))
    return false;

  // Try to heapify in both directions (only one will).
  heapify_element(inverse[index]);

  return true;
}

/**
 * Rebuild the heap structure completely. Designed to be used when many elements have been modified.
 * Cost: O(n), better than O(n log n) from n individual updates.
*/
template <typename T, typename C, typename idx>
void indirect_heap<T, C, idx>::updateAll() {

  // Reset indices.
  for (unsigned int i=1; i<=size; ++i)
    inverse[i] = heap[i] = i;

  // Rebuild heap.
  for (unsigned int i=(used >> 1); i>=root; --i)
    heapify_downwards(i);
}

/**
 * Update the heap after two elements were swapped from the data array. Cost: O(1).
 *
 * \param index1 Index of the first element in the data array (0-indexed).
 * \param index2 Index of the second element in the data array (0-indexed).
 * \return \c true if successful, \c false if any of the indices is not valid or not in the heap.
*/
template <typename T, typename C, typename idx>
bool indirect_heap<T, C, idx>::swap(idx index1, idx index2) {

  // Check if both indices are in the heap.
  if (!inHeap(index1++))
    return false;
  if (!inHeap(index2++))
    return false;

  // Swap elements (works inverse to swap_elements internal method).
  std::swap<idx>(heap[inverse[index1]], heap[inverse[index2]]);
  std::swap<idx>(inverse[index1], inverse[index2]);

  return true;
}

/**
 * Get the data array index of the element being currently in the top of the heap.
 *
 * \return 0-based index of the topmost element in the heap, or first one if the heap is \link indirect_heap::empty empty\endlink.
*/
template <typename T, typename C, typename idx>
idx indirect_heap<T, C, idx>::topIndex() const {
  if (empty())
    return 0;
  return heap[root] - 1;
}

/**
 * Get the element in the top of the heap.
 *
 * \return Reference to the element in the top of the heap, or to the first one if the heap is \link indirect_heap::empty empty\endlink.
*/
template <typename T, typename C, typename idx>
T &indirect_heap<T, C, idx>::top() const {
  if (empty())
    return const_cast<T *>(data)[root];
  return const_cast<T *>(data)[heap[root]];
}

/**
 * Check if the heap is empty.
 *
 * \return \c true if empty, \c false if not.
*/
template <typename T, typename C, typename idx>
bool indirect_heap<T, C, idx>::empty() const {
  return used == 0;
}

/**
 * Get maximum heap size.
 *
 * \return Maximum heap size.
*/
template <typename T, typename C, typename idx>
unsigned int indirect_heap<T, C, idx>::maxSize() const {
  return size;
}

/**
 * Get the current number of elements in the data array (may be increased up to \link indirect_heap::maxSize maxSize\endlink when \link indirect_heap::push pushing\endlink data).
 *
 * \return Current number of elements in the data array.
*/
template <typename T, typename C, typename idx>
unsigned int indirect_heap<T, C, idx>::dataSize() const {
  return last;
}


/**
 * Get the number of elements in the heap.
 *
 * \return Current number of elements in the heap.
*/
template <typename T, typename C, typename idx>
unsigned int indirect_heap<T, C, idx>::count() const {
  return used;
}
