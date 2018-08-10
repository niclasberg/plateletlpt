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
 * \file kd-tree_sse_24d.h
 * \brief Distance function specializations for single-precision floating point vectors of 24 dimensions using the SSE instruction set.
 * \author Leandro Graciá Gil
*/

#ifndef _KD_TREE_SSE_24D_H_
#define _KD_TREE_SSE_24D_H_

// Includes for the SSE instruction set and aligned memory allocation.
#include <xmmintrin.h>
#include <malloc.h>

// Include the kd-tree template.
#include "kd-tree.h"

/**
 * Memory-aligned version of the memory allocation operator for feature vectors.
 * Ensures the 16-byte alignment required by the SSE instructions.
 *
 * \param size Total size in bytes of the objects to allocate.
 * \return Address to the new allocated memory.
*/
template <>
void *feature_vector<float, 24U>::operator new [] (size_t size) {

  // Allocate memory aligned to 16 bytes.
  void *p = memalign(16, size);

  // Throw an allocation exception in case of error.
  if (p == NULL) {
    std::bad_alloc exception;
    throw exception;
  }

  return p;
}

/**
 * SSE-accelerated squared euclidean distance operator for two 24-dimensional single precision kd_points.
 *
 * \warning All feature vectors must be allocated aligned to 16 bytes.
 *
 * \param p Point being compared to.
 * \return Euclidean squared distance between the two kd_points.
*/
template <>
float feature_vector<float, 24U>::distance_to(const feature_vector<float, 24U> &p) const {

  // Set data pointers as SSE vectors. Let's say each is composed of 6 4-float vectors: A B C D E F.
  __m128 *this_data = (__m128 *) data;
  __m128 *p_data = (__m128 *) p.data;

  // Required SSE registers.
  __m128 sqr1, sqr2, acc1, acc2;

  // Calculate acc1 = dist(A + B).
  sqr1 = _mm_sub_ps(this_data[0], p_data[0]);
  sqr2 = _mm_sub_ps(this_data[1], p_data[1]);
  sqr1 = _mm_mul_ps(sqr1, sqr1);
  sqr2 = _mm_mul_ps(sqr2, sqr2);
  acc1 = _mm_add_ps(sqr1, sqr2);

  // Calculate acc2 = dist(C + D).
  sqr1 = _mm_sub_ps(this_data[2], p_data[2]);
  sqr2 = _mm_sub_ps(this_data[3], p_data[3]);
  sqr1 = _mm_mul_ps(sqr1, sqr1);
  sqr2 = _mm_mul_ps(sqr2, sqr2);
  acc2 = _mm_add_ps(sqr1, sqr2);

  // Accumulate acc1 += acc2 (acc1 contains A + B + C + D).
  acc1 = _mm_add_ps(acc1, acc2);

  // Calculate acc2 = dist(E + F).
  sqr1 = _mm_sub_ps(this_data[4], p_data[4]);
  sqr2 = _mm_sub_ps(this_data[5], p_data[5]);
  sqr1 = _mm_mul_ps(sqr1, sqr1);
  sqr2 = _mm_mul_ps(sqr2, sqr2);
  acc2 = _mm_add_ps(sqr1, sqr2);

  // Accumulate acc1 += acc2 (acc1 contains A + B + C + D + E + F).
  acc1 = _mm_add_ps(acc1, acc2);

  // Return the sum of the 4 acc1 elements.
  const float *f = (const float *) &acc1;
  return f[0] + f[1] + f[2] + f[3];
}

/**
 * SSE-accelerated squared euclidean distance operator for two 24-dimensional single precision kd_points.
 * Special version with early leaving in case an upper bound value is reached.
 *
 * \warning All feature vectors must be allocated aligned to 16 bytes.
 *
 * \param p Point being compared to.
 * \param upper Upper bound for the distance. Will return immediatly if reached.
 * \return Euclidean squared distance between the two kd_points or a partial result greater or equal than \a upper.
*/
template <>
float feature_vector<float, 24U>::distance_to(const feature_vector<float, 24U> &p, float upper) const {

  // Set data pointers as SSE vectors. Let's say each is composed of 6 4-float vectors: A B C D E F.
  __m128 *this_data = (__m128 *) data;
  __m128 *p_data = (__m128 *) p.data;

  // Required SSE registers.
  __m128 sqr1, sqr2, acc1, acc2;

  // Calculate acc1 = dist(A + B).
  sqr1 = _mm_sub_ps(this_data[0], p_data[0]);
  sqr2 = _mm_sub_ps(this_data[1], p_data[1]);
  sqr1 = _mm_mul_ps(sqr1, sqr1);
  sqr2 = _mm_mul_ps(sqr2, sqr2);
  acc1 = _mm_add_ps(sqr1, sqr2);

  // Calculate acc2 = dist(C + D).
  sqr1 = _mm_sub_ps(this_data[2], p_data[2]);
  sqr2 = _mm_sub_ps(this_data[3], p_data[3]);
  sqr1 = _mm_mul_ps(sqr1, sqr1);
  sqr2 = _mm_mul_ps(sqr2, sqr2);
  acc2 = _mm_add_ps(sqr1, sqr2);

  // Accumulate acc1 += acc2 (acc1 contains A + B + C + D).
  acc1 = _mm_add_ps(acc1, acc2);

  // Check upper bound.
  const float *f = (const float *) &acc1;
  float partial_acc = f[0] + f[1] + f[2];
  if (partial_acc > upper)
    return partial_acc;

  // Calculate acc2 = dist(E + F).
  sqr1 = _mm_sub_ps(this_data[4], p_data[4]);
  sqr2 = _mm_sub_ps(this_data[5], p_data[5]);
  sqr1 = _mm_mul_ps(sqr1, sqr1);
  sqr2 = _mm_mul_ps(sqr2, sqr2);
  acc2 = _mm_add_ps(sqr1, sqr2);

  // Accumulate acc1 += acc2 (acc1 contains A + B + C + D + E + F).
  acc1 = _mm_add_ps(acc1, acc2);

  // Return the sum of the 4 acc1 elements.
  return f[0] + f[1] + f[2] + f[3];
}

#endif
