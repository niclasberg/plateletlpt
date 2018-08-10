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
 * \file k-heap.cpp
 * \brief Template implementations for k-heaps holding the best k elements.
 * \author Leandro Graciá Gil
*/

#include <algorithm>
#include <cassert>
#include <cstring>

using std::swap;

/**
 * Build a k-heap of the given \a K size.
 *
 * \param K Maximum number of best elements to store in the heap.
*/
template <typename T, typename C, typename idx>
k_heap<T, C, idx>::k_heap(unsigned int K)
  : K(K),
    data(new T[K]),
    compare(C()),
    bestHeap(data, 0, K, compare),
    worstHeap(data, 0, K, std::binary_negate<C>(compare)) {}

/**
 * \brief Copy constructor.
 *
 * Initialize a k_heap copying another one.
 *
 * \param heap Element being copied.
 */
template <typename T, typename C, typename idx>
k_heap<T, C, idx>::k_heap(const k_heap &heap)
  : data(NULL),
    compare(heap.compare),
    bestHeap(NULL, 0),
    worstHeap(NULL, 0, 0, std::binary_negate<C>(compare)) {

  // Use assignment operator.
  *this = heap;
}

/// Default destructor.
template <typename T, typename C, typename idx>
k_heap<T, C, idx>::~k_heap() {

  // Release data.
  delete []data;
}

/**
 * \brief Assignment operator.
 *
 * Copy the contents from another heap.
 *
 * \param heap Element being copied.
 */
template <typename T, typename C, typename idx>
k_heap<T, C, idx> &k_heap<T, C, idx>::operator = (const k_heap &heap) {

  // Check self assignment.
  if (this == &heap)
    return *this;

  // Set size.
  K = heap.K;

  // Reallocate data.
  if (data)
    delete []data;
  assert(data = new T[K]);

  // Copy data.
  memcpy(data, heap.data, K * sizeof(T));
  bestHeap.setData(data);
  worstHeap.setData(data);

  // Copy heap structure.
  bestHeap = heap.bestHeap;
  worstHeap = heap.worstHeap;

  // Return a reference to itself.
  return *this;
}

/**
 * \brief Comparison operator.
 *
 * Check if two heaps are identical in data an structure.
 *
 * \return \c true if equal, \c false otherwise.
 */
template <typename T, typename C, typename idx>
bool k_heap<T, C, idx>::operator == (const k_heap &heap) const {

  // Compare K value.
  if (K != heap.K)
    return false;

  // Compare data.
  for (unsigned int i=0; i<K; ++i)
    if (!(data[i] == heap.data[i]))
      return false;

  // Compare internal heaps.
  if (!(bestHeap == heap.bestHeap))
    return false;
  if (!(worstHeap == heap.worstHeap))
    return false;

  return true;
}

/**
 * Insert an element in the k-heap.
 *
 * \param elem Element to insert.
 * \return \c true if \a elem is kept in the heap, \c false if discarded.
*/
template <typename T, typename C, typename idx>
bool k_heap<T, C, idx>::push(const T &elem) {

  // Check if heaps are not yet full.
  if (bestHeap.count() < K) {

    // Insert element into data vector.
    idx next = bestHeap.count();
    data[next] = elem;

    // Insert element in both heaps.
    bestHeap.push(next);
    worstHeap.push(next);

    return true;
  }
  // Heaps are full: should try to replace worst element.
  else {
    // Check if new element is better than the actual worst one.
    if (compare(elem, worst())) {

      // Replace worst element in the array.
      idx worstIndex = worstHeap.topIndex();
      data[worstIndex] = elem;

      // Update heaps.
      bestHeap.update(worstIndex);
      worstHeap.update(worstIndex);

      return true;
    }
    // Discard new element.
    return false;
  }
}

/**
 * Extract the best element from the heap.
 * No value is returned to avoid unnecessary object copies.
*/
template <typename T, typename C, typename idx>
void k_heap<T, C, idx>::pop_best() {

  // Check size.
  if (bestHeap.empty())
    return;

  // Get the index of the best and the last elements.
  idx bestIndex = bestHeap.topIndex();
  idx lastIndex = bestHeap.count() - 1;

  // Swap extracted element to the end in the data array and update heaps.
  if (bestIndex != lastIndex) {
    swap(data[bestIndex], data[lastIndex]);
    bestHeap.swap(bestIndex, lastIndex);
    worstHeap.swap(bestIndex, lastIndex);
    swap(bestIndex, lastIndex);
  }

  // Remove the element from the worst elements heap.
  worstHeap.remove(bestIndex);

  // Extract the topmost object from the best elements heap.
  bestHeap.pop();
}

/**
 * Extract the worst element from the heap.
 * No value is returned to avoid unnecessary object copies.
*/
template <typename T, typename C, typename idx>
void k_heap<T, C, idx>::pop_worst() {

  // Check size.
  if (worstHeap.empty())
    return;

  // Get the index of the worst element.
  idx worstIndex = worstHeap.topIndex();
  idx lastIndex = worstHeap.count() - 1;

  // Swap extracted element to the end in the data array and update heaps.
  if (worstIndex != lastIndex) {
    swap(data[worstIndex], data[lastIndex]);
    bestHeap.swap(worstIndex, lastIndex);
    worstHeap.swap(worstIndex, lastIndex);
    swap(worstIndex, lastIndex);
  }

  // Remove the element from the best elements heap.
  bestHeap.remove(worstIndex);

  // Extract the topmost object from the worst elements heap.
  worstHeap.pop();
}

/**
 * Check if the heap is full.
 *
 * \return \c true if full, \c false if not.
*/
template <typename T, typename C, typename idx>
bool k_heap<T, C, idx>::full() const {
  return bestHeap.count() == K;
}

/**
 * Check if the heap is empty.
 *
 * \return \c true if empty, \c false if not.
*/
template <typename T, typename C, typename idx>
bool k_heap<T, C, idx>::empty() const {
  return bestHeap.empty();
}

/**
 * Get the number of elements in the heap.
 *
 * \return Number of elements in the heap.
*/
template <typename T, typename C, typename idx>
unsigned int k_heap<T, C, idx>::size() const {
  return bestHeap.count();
}

/**
 * Get the current best element in the heap.
 *
 * \return Best element in the heap, or last 'first' element if heap is already \link k_heap::empty empty\endlink.
*/
template <typename T, typename C, typename idx>
const T &k_heap<T, C, idx>::best() const {
  if (empty())
    return data[0];
  return bestHeap.top();
}

/**
 * Get the current worst element in the heap.
 *
 * \return Worst element in the heap, or last 'first' element if heap is already \link k_heap::empty empty\endlink.
*/
template <typename T, typename C, typename idx>
const T &k_heap<T, C, idx>::worst() const {
  if (empty())
    return data[0];
  return worstHeap.top();
}

/**
 * \brief Get the maximum number of elements stored in the heap (also the maximum heap size). Cost: O(1).
 *
 * \return The K value used to build the heap.
*/
template <typename T, typename C, typename idx>
unsigned int k_heap<T, C, idx>::get_K() const {
  return K;
}

/**
 * \brief Get the number of elements currently in the heap. Cost: O(1).
 *
 * \return Current number of elements in the heap.
*/
template <typename T, typename C, typename idx>
unsigned int k_heap<T, C, idx>::count() const {
  return bestHeap.count();
}
