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
 * \file feature_vector.cpp
 * \brief Template implementations for generic D-dimensional feature vectors.
 * \author Leandro Graciá Gil
 */

// Includes from STL and the C standard library.
#include <cstdlib>
#include <new>

/**
 * Generic memory allocator operator for feature vector arrays.
 * Defined so that memory-aligned specializations can be defined if required.
 *
 * \param size Total size in bytes of the objects to allocate.
 * \return Address to the new allocated memory.
 */
template <typename T, const unsigned int D>
void *feature_vector<T, D>::operator new [] (size_t size) {

  // Allocate plain memory for the requested vectors.
  void *p = malloc(size);

  // Throw an allocation exception in case of error.
  if (p == NULL) {
    std::bad_alloc exception;
    throw exception;
  }

  return p;
}

/**
 * Generic memory deallocator operator for feature vector arrays.
 * Defined as complement of operator new [] so that memory-aligned specializations can be defined if required.
 *
 * \param p Pointer to the address to release.
 */
template <typename T, const unsigned int D>
void feature_vector<T, D>::operator delete [] (void *p) {

  // Just release memory.
  free(p);
}

/**
 * Check if two feature vectors are exactly equal.
 *
 * \param p Feature vector being compared to.
 * \return \c true if equal, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool feature_vector<T, D>::operator == (const feature_vector &p) const {

  // Check that all dimensions have the same value.
  for (unsigned int d=0; d<D; ++d) {
    if (data[d] != p.data[d])
      return false;
  }
  return true;
}

/**
 * Check if two feature vectors are different equal.
 *
 * \param p Feature vector being compared to.
 * \return \c true if different, \c false otherwise.
 */
template <typename T, const unsigned int D>
bool feature_vector<T, D>::operator != (const feature_vector &p) const {

  // Check if any dimension has a different value.
  for (unsigned int d=0; d<D; ++d) {
    if (data[d] != p.data[d])
      return true;
  }
  return false;
}

/**
 * Generic squared euclidean distance operator for two D-dimensional numeric kd_points.
 * Redefinitions and specializations of this operator are welcome.
 *
 * \param p Point being compared to.
 * \return Euclidean squared distance between the two kd_points.
 */
template <typename T, const unsigned int D>
T feature_vector<T, D>::distance_to(const feature_vector &p) const {

  // Standard squared distance between two D-dimensional vectors.
  T acc = (T) 0;
  for (unsigned int i=0; i<D; ++i)
    acc += (data[i] - p[i]) * (data[i] - p[i]);
  return acc;
}

/**
 * Generic squared euclidean distance operator for two D-dimensional numeric kd_points.
 * Special version with early leaving in case an upper bound value is reached.
 *
 * \param p Point being compared to.
 * \param upper_bound Upper bound for the distance. Will return immediatly if reached.
 * \return Euclidean squared distance between the two kd_points or a partial result greater or equal than \a upper.
 */
template <typename T, const unsigned int D>
T feature_vector<T, D>::distance_to(const feature_vector &p, T upper_bound) const {

  // Constant calculated empirically.
  const unsigned int D_acc = (unsigned int) (0.25f * D);

  // Squared distance in two steps: first accumulate without comparisons.
  T acc = (T) 0;
  for (unsigned int i=0; i<D_acc; ++i)
    acc += (data[i] - p[i]) * (data[i] - p[i]);

  // Second step: accumulate comparing with upper bound.
  for (unsigned int i=D_acc; i<D; ++i) {
    acc += (data[i] - p[i]) * (data[i] - p[i]);
    if (acc > upper_bound)
      break;
  }

  return acc;
}
