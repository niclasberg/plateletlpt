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
 * \file k-vector.cpp
 * \brief Template implementations for k-vectors holding the best k elements (linear, but simpler).
 * \author Leandro Graciá Gil
*/

/**
 * Build a k-vector of size K.
 *
 * \param K Size of the k-vector.
 * \param c Comparison object used for internal element sorting.
 */
template <typename T, typename Compare>
k_vector<T, Compare>::k_vector(unsigned int K, const Compare &c)
  : data(new T[K]),
    K(K),
    stored(0),
    compare(c) {}

/** Default destructor */
template <typename T, typename Compare>
k_vector<T, Compare>::~k_vector() {
  delete []data;
}

/**
 * Check if no elements are being stored in the k-vector.
 *
 * \return \c true if the k-vector is empty, \c false otherwise.
 */
template <typename T, typename Compare>
bool k_vector<T, Compare>::empty() const {
  return stored == 0;
}

/**
 * Check if K elements are being stored in the k-vector with K being the maximum size.
 *
 * \return \c true if the k-vector is full, \c false otherwise.
 */
template <typename T, typename Compare>
bool k_vector<T, Compare>::full() const {
  return stored == K;
}

/**
 * Get the current number of elements in the k-vector.
 *
 * \return Number of elements currently in the k-vector.
 */
template <typename T, typename Compare>
unsigned int k_vector<T, Compare>::size() const {
  return stored;
}

/**
 * Get the worst element currently stored in the k-vector.
 *
 * \return Reference to the worst element stored.
 */
template <typename T, typename Compare>
const T & k_vector<T, Compare>::front() const {
  return data[0];
}

/**
 * Get the best element currently stored in the k-vector.
 *
 * \return Reference to the best element stored.
 */
template <typename T, typename Compare>
const T & k_vector<T, Compare>::back() const {
  return data[stored - 1];
}

/**
 * Pop the best element stored in the k-vector.
 */
template <typename T, typename Compare>
void k_vector<T, Compare>::pop_back() {
  if (stored > 0)
    --stored;
}

/**
 * Push a new element into the k-vector.
 *
 * \param elem Element being pushed.
 */
template <typename T, typename Compare>
void k_vector<T, Compare>::push_back(const T &elem) {
  if (stored < K)
    push_not_full(elem);
  else
    push_full(elem);
}

/**
 * Push a new element into the k-vector with the precondition that it's still not full.
 * Ensuring the precondition before calling is responsability of the caller.
 * For a generic pushing method use \link k_vector::push_back push_back\endlink.
 *
 * \pre The k-vector object must not be full.
 *
 * \param elem Element being pushed.
 */
template <typename T, typename Compare>
void k_vector<T, Compare>::push_not_full(const T &elem) {

  // Look where the new candidate should be placed.
  unsigned int index;
  for (index=0; index < stored && compare(elem, data[index]); ++index);

  // Move later candidates and store the new one in its place.
  for (unsigned int i=stored; i>index; --i)
    data[i] = data[i-1];
  data[index] = elem;
  ++stored;
}

/**
 * Push a new element into the k-vector with the precondition that it's already full.
 * Ensuring the precondition before calling is responsability of the caller.
 * For a generic pushing method use \link k_vector::push_back push_back\endlink.
 *
 * \pre The k-vector object must be already full.
 *
 * \param elem Element being pushed.
 */
template <typename T, typename Compare>
void k_vector<T, Compare>::push_full(const T &elem) {

  // Avoid further calculations if candidate is worst than the current worst one.
  if (!compare(elem, data[0]))
    return;

  // Look where the new candidate should be placed.
  unsigned int index;
  for (index=1; index < stored && compare(elem, data[index]); ++index);

  // Move previous candidates and store the new one in its place.
  --index;
  for (unsigned int i=0; i<index; ++i)
    data[i] = data[i+1];
  data[index] = elem;
}
