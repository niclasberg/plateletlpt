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
 * \file k-vector.h
 * \brief Template for k-vectors holding the best k elements (linear).
 * \author Leandro Graciá Gil
*/

#ifndef _K_VECTOR_H_
#define _K_VECTOR_H_

// Include STL less comparison template (used by default).
#include <functional>

/**
 * \brief Define an insertion-based vector to hold the K best elements pushed.
 *
 * \note Element insertions are asyntotically linear. However they may be faster than \link k_heap k-heaps\endlink for small K values.
 *
 * \tparam T Type of the data stored in the k-vector.
 * \tparam Compare Type of the comparison object. Defaults to std::less<T>.
 */
template <typename T, typename Compare = std::less<T> >
class k_vector {
public:
  /// K-vector constructor.
  k_vector(unsigned int K, const Compare &c = Compare());

  /// Default destructor.
  ~k_vector();

  // K-vector properties.
  bool empty() const; ///< Check if the k-vector is empty. Cost: O(1).
  bool full() const; ///< Check if the k-vector is full (has K elements). Cost: O(1).
  unsigned int size() const; ///< Get the number of elements currently in the k-vector. Cost: O(1).

  const T &front() const; ///< Get the worst element stored in the k-vector. Cost: O(1).
  const T &back() const; ///< Get the best element stored in the k-vector. Cost: O(1).

  // Operations.
  void pop_back(); ///< Pop the current worst element from the k-vector. Cost: O(K).
  void push_back(const T &elem); ///< Push a new element into the k-vector. Cost: O(K).

protected:
  T *data; ///< Array of stored elements (sorted with worst one on the first position).
  unsigned int K; ///< Maximum number of best elements stored (also maximum heap size).
  unsigned int stored; ///< Number of elements currently in the k-vector.
  const Compare &compare; ///< Comparison object.

  void push_not_full(const T &elem); ///< Push a new element into the k-vector when the vector is still not full. Cost: O(K).
  void push_full(const T &elem); ///< Push a new element into the k-vector when the vector is already full. Cost: O(K).
};

// Template implementation.
#include "k-vector.cpp"

#endif
