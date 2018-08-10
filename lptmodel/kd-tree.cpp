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
 * \file kd-tree.cpp
 * \brief Template implementations for generic kd-trees.
 * \author Leandro Graciá Gil
 */

// Includes from STL and C standard library.
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <new>

/**
 * Default kd-tree constructor.
 * Creates an empty and uninitialized kd-tree.
 */
template <typename T, const unsigned int D, typename S>
kd_tree<T, D, S>::kd_tree()
  : root(NULL),
    data(NULL),
    permutation(NULL),
    inverse_perm(NULL),
    num_elements(0) {}

/**
 * Default kd-tree desstructor.
 */
template <typename T, const unsigned int D, typename S>
kd_tree<T, D, S>::~kd_tree() {

  // Delete allocated contents.
  release();
}

/**
 * Release any existing contents in the kd-tree.
 */
template <typename T, const unsigned int D, typename S>
void kd_tree<T, D, S>::release() {

  // Release tree nodes and data arrays.
  delete root;
  delete []data;
  delete []permutation;
  delete []inverse_perm;

  // Set NULL pointers.
  root = NULL;
  data = NULL;
  permutation = NULL;
  inverse_perm = NULL;

  // Reset number of elements.
  num_elements = 0;
}

/**
 * Subscripting operator to access kd-tree data.
 *
 * \param index Index of the data being accessed.
 * \return Reference to the index-th element provided when building the tree (internal copy).
 */
template <typename T, const unsigned int D, typename S>
const typename kd_tree<T, D, S>::kd_point & kd_tree<T, D, S>::operator [] (unsigned int index) const {
  return data[inverse_perm[index]];
}

/**
 * Build a kd-tree from a set of \a n D-dimensional samples.
 *
 * \param points Array of contiguous D-dimensional kd_points.
 * \param num_points Number of elements in \a p.
 * \param bucket_size Number of elements that should be grouped in leaf nodes.
 * \return \c true if successful, \c false otherwise.
 */
template <typename T, const unsigned int D, typename S>
bool kd_tree<T, D, S>::build(const kd_point *points, unsigned int num_points, unsigned int bucket_size) {

  // Check params.
  if (points == NULL || num_points == 0 || bucket_size == 0)
    return false;

  // Delete any previous kd-tree.
  release();

  // Prepare an array for data index permutations.
  permutation = new unsigned int[num_points];
  for (unsigned int i=0; i<num_points; ++i)
    permutation[i] = i;

  // Build the kd-tree recursively (num_elements will contain a recursively-calculated num_points after the call).
  num_elements = 0;
  root = kd_node::build(points, permutation, num_points, NULL, bucket_size, num_elements);

  // Make a permutated copy of the input data and calculate the inverse permutation.
  data = new kd_point[num_points];
  inverse_perm = new unsigned int[num_points];
  for (unsigned int i=0; i<num_points; ++i) {
    data[i] = points[permutation[i]];
    inverse_perm[permutation[i]] = i;
  }

  return true;
}

/**
 * Build a kd-tree recursively.
 *
 * \param data Base of the data array.
 * \param indices Array of indices to D-dimensional data vectors.
 * \param n Number of elements in \a index.
 * \param parent Parent node.
 * \param bucket_size Number of elements that should be grouped in leaf nodes.
 * \param processed Number of elements already processed and stored in the tree. Updated as the building expands.
 * \return Node of the tree completely initialized.
 */
template <typename T, const unsigned int D, typename S>
typename kd_tree<T, D, S>::kd_node *kd_tree<T, D, S>::kd_node::build(const kd_point *data, unsigned int *indices, unsigned int n,
    kd_node *parent, unsigned int bucket_size, unsigned int &processed) {

  // Handle empty nodes (only for degenerate bucket sizes).
  if (n == 0)
    return NULL;

  // Allocate a new node.
  kd_node *node = new kd_node();

  // Split the data with a basic cycle over the dimension indices.
  node->axis = parent ? (parent->axis + 1) % D : 0;

  // Create a sorter for the current axis.
  kd_axis_comparer comparer = { data, node->axis };

  // Find a pivot to split data appropiately (may involve index sorting or partitioning).
  unsigned int pivot = node->split(indices, n, comparer);

  // Split the data in two segments: left to pivot inclusive, and elements right to it.
  unsigned int left_elements = pivot + 1;
  unsigned int right_elements = n - left_elements;
  unsigned int *right_indices = indices + left_elements;

  // Store the axis-th value of the pivot used to split the hyperspace in two.
  node->split_value = data[indices[pivot]][node->axis];

  // Process the left part recursively, creating a leaf is remaining data is not greater than the bucket size.
  if (left_elements > bucket_size)
    node->left_branch = build(data, indices, left_elements, node, bucket_size, processed);
  else {
    node->left_leaf = new kd_leaf(processed, left_elements);
    node->is_leaf |= left_bit;
    processed += left_elements;
  }

  // Process the right part recursively, creating a leaf is remaining data is not greater than the bucket size.
  if (right_elements > bucket_size)
    node->right_branch = build(data, right_indices, right_elements, node, bucket_size, processed);
  else {
    node->right_leaf = new kd_leaf(processed, right_elements);
    node->is_leaf |= right_bit;
    processed += right_elements;
  }

  // Return processed node.
  return node;
}

/**
 * Split the provided data subset by one dimension. Should be near to the median to get a balanced kd-tree.
 * The dimension used to split the data is also decided by this method.
 * Any index sorting or partitioning must be also performed here.
 *
 * \param indices Array of indices to elements of the current data subset.
 * \param n Number of elements in \a index.
 * \param comparer Functor object used to compare data elements. Can be used with STL comparison-based algorithms.
 *
 * \return The index of the pivot element in the index array used to split the space.
 * All data in the left half must be less than the value associated to this index.
 */
template <typename T, const unsigned int D, typename S>
unsigned int kd_tree<T, D, S>::kd_node::split(unsigned int *indices, unsigned int n, const kd_axis_comparer &comparer) {

  // Avoid sorting when less than 2 elements (base case).
  if (n < 2)
    return 0;

  // Sort the indices (a partial sort was used previously, but it was slower due to the internal use of STL heaps).
  std::sort(indices, indices + n, comparer);

  // Return the index of the median.
  unsigned int median = ((n + 1) >> 1) - 1;
  return median;
}

/**
 * Default node destructor.
 * Delete the tree recursively handling branches and leaf nodes appropiately.
 */
template <typename T, const unsigned int D, typename S>
kd_tree<T, D, S>::kd_node::~kd_node() {

  // Delete left branch / leaf.
  if (is_leaf & left_bit)
    delete left_leaf;
  else
    delete left_branch;

  // Delete right branch / leaf.
  if (is_leaf & right_bit)
    delete right_leaf;
  else
    delete right_branch;
}

/**
 * Find the K nearest neighbours of a given kd_point and push their indices sorted into a given STL vector.
 * In case that there are not enough kd_points in the tree, all the available ones will be provided.
 *
 * \param p Point whose \a K neighbours should be retrieved.
 * \param K Number of nearest neighbours to retrieve.
 * \param output STL vector where the nearest neighbours will be appended. Sorted result depends on the template parameter S, enabled by default.
 * \param epsilon Acceptable distance margin to ignore regions during kd-tree exploration. Defaults to zero (deterministic).
 * \param ignore_p_in_tree Assume that \a p is contained in the tree any number of times and ignore them all.
 */
template <typename T, const unsigned int D, typename S>
void kd_tree<T, D, S>::knn(const kd_point &p, unsigned int K, std::vector<kd_neighbour> &output, T epsilon, bool ignore_p_in_tree) const {

  // Check if there is any data on the tree and K is valid.
  if (root == NULL || num_elements == 0 || K == 0)
    return;

  // Create an object for tree traversal and incremental hyperrectangle-hypersphere intersection calculation.
  kd_search_data search_data(p, data, K, ignore_p_in_tree);

  // Convert epsilon to a squared distance and set it as initial hyperrectangle distance.
  search_data.hyperrect_distance = epsilon * epsilon;

  // Build a special sorted container for the current K nearest neighbour candidates.
  S best_k(K);

  // Start an exploration traversal from the root.
  root->explore(NULL, search_data, best_k);

  // Append the nearest neighbours to the output vector in increasing distance correcting index permutations.
  while (!best_k.empty()) {
    kd_neighbour neighbour = best_k.back();
    neighbour.index = permutation[neighbour.index];
    output.push_back(neighbour);
    best_k.pop_back();
  }
}

/**
 * Find the K nearest neighbours of a given kd_point and push their indices sorted into a given STL vector.
 * In case that there are not enough kd_points in the tree, all the available ones will be provided.
 *
 * \param p Point whose \a K neighbours should be retrieved.
 * \param distance Euclidean distance margin used to retrieve all points within.
 * \param output STL vector where the neighbours within the specified range will be appended. Elements are not sorted by distance.
 * \param ignore_p_in_tree Assume that \a p is contained in the tree any number of times and ignore them all.
 */
template <typename T, const unsigned int D, typename S>
void kd_tree<T, D, S>::all_in_range(const kd_point &p, T distance, std::vector<kd_neighbour> &output, bool ignore_p_in_tree) const {

  // Check if there is any data on the tree and K is valid.
  if (root == NULL || num_elements == 0 || distance <= (T) 0)
    return;

  // Create an object for tree traversal and incremental hyperrectangle-hypersphere intersection calculation.
  kd_search_data search_data(p, data, 0, ignore_p_in_tree);
  search_data.farthest_distance = distance * distance;

  // Build a STL vector to hold all the points in range.
  std::vector<kd_neighbour> points_in_range;

  // Store a dummy element in the vector with the distance range (will act as the farthest nearest neighbour during calculations).
  points_in_range.push_back(kd_neighbour(-1, search_data.farthest_distance));

  // Start an exploration traversal from the root.
  root->explore(NULL, search_data, points_in_range);

  // Append the nearest neighbours to the output vector correcting index permutations.
  for (unsigned int i=1; i<points_in_range.size(); ++i) {
    kd_neighbour neighbour = points_in_range[i];
    neighbour.index = permutation[neighbour.index];
    output.push_back(neighbour);
  }
}

/**
 * Initialize a data searching structure with incremental hyperrectangle intersection calculation.
 *
 * \param p Reference point being used in the search.
 * \param data Array of permutated data points.
 * \param K Number of neighbours to retrieve.
 */
template <typename T, const unsigned int D, typename S>
kd_tree<T, D, S>::kd_search_data::kd_search_data(const kd_point &p, const kd_point *data, unsigned int K, bool ignore_null_distances_arg)
  : p(p),
    data(data),
    K(K),
    hyperrect_distance((T) 0),
    farthest_distance((T) 0),
    ignore_null_distances(ignore_null_distances_arg) {

  // Fill per-axis data contents.
  for (unsigned int d=0; d<D; ++d) {
    axis[d].p = p[d];
    axis[d].nearest = p[d];
  }
}

/**
 * Apply the incremental operations corresponding to traverse the tree by its left branch.
 *
 * \param node Current node in the sub-hyperrectangular region.
 * \param parent Parent node that halves the hyperspace in two.
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 */
template <typename T, const unsigned int D, typename S>
kd_tree<T, D, S>::kd_incremental::kd_incremental(const kd_node *node, const kd_node *parent, kd_search_data &search_data)
  : search_data(search_data),
    parent_axis(0),
    modified(false),
    previous_axis_nearest((T) 0),
    previous_hyperrect_distance((T) 0) {

  // Check parent.
  if (parent == NULL)
    return;

  // Get splitting axis data.
  parent_axis = parent->axis & kd_node::axis_mask;
  typename kd_search_data::axis_data *axis = &search_data.axis[parent_axis];

  // Check if current branch modifies the bounding hyperrectangle.
  if ((parent->left_branch  == node && parent->split_value > axis->nearest) ||
      (parent->right_branch == node && parent->split_value < axis->nearest))
    return;

  // Store current values before any update.
  modified = true;
  previous_axis_nearest = axis->nearest;
  previous_hyperrect_distance = search_data.hyperrect_distance;

  // Perform incremental update (simplification of the equation local * (local + 2 * (p - nearest)) with local = nearest - split).
  search_data.hyperrect_distance += (parent->split_value - axis->nearest) * (axis->nearest + parent->split_value - axis->p * 2);
  axis->nearest = parent->split_value;
}

/**
 * Restore the updated values to their previous ones, if modified.
 */
template <typename T, const unsigned int D, typename S>
kd_tree<T, D, S>::kd_incremental::~kd_incremental() {

  // Restore previous values if modified.
  if (modified) {
    search_data.axis[parent_axis].nearest = previous_axis_nearest;
    search_data.hyperrect_distance = previous_hyperrect_distance;
  }
}

/**
 * Traverse the kd-tree looking for nearest neighbours candidates, but do not discard any regions of the space.
 *
 * \param parent Parent node of the one being explored.
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D, typename S> template <typename C>
void kd_tree<T, D, S>::kd_node::explore(const kd_node *parent, kd_search_data &search_data, C &candidates) const {

  // Intersection data is updated incrementally when the object is created, and restored when destroyed.
  kd_incremental incremental_update(this, parent, search_data);

  // Check which branch should be explored first.
  kd_node *first_branch = NULL, *second_branch = NULL;
  kd_leaf *first_leaf = NULL, *second_leaf = NULL;

  // Left branch first or same point.
  if (search_data.p[axis & axis_mask] <= split_value) {
    if (is_leaf & left_bit)
      first_leaf = left_leaf;
    else
      first_branch = left_branch;

    if (is_leaf & right_bit)
      second_leaf = right_leaf;
    else
      second_branch = right_branch;
  }
  // Right branch first.
  else {
    if (is_leaf & left_bit)
      first_leaf = right_leaf;
    else
      first_branch = right_branch;

    if (is_leaf & right_bit)
      second_leaf = left_leaf;
    else
      second_branch = left_branch;
  }

  // Traverse the first (manhattan nearest) branch.
  bool full = candidates.size() >= search_data.K;
  if (first_leaf != NULL) {
    if (full)
      first_leaf->intersect(search_data, candidates);
    else
      first_leaf->explore(search_data, candidates);
  }
  else {
    if (full)
      first_branch->intersect(this, search_data, candidates);
    else
      first_branch->explore(this, search_data, candidates);
  }

  // Traverse the second (manhattan farthest) branch.
  full = candidates.size() >= search_data.K;
  if (second_leaf != NULL) {
    if (full)
      second_leaf->intersect(search_data, candidates);
    else
      second_leaf->explore(search_data, candidates);
  }
  else {
    if (full)
      second_branch->intersect(this, search_data, candidates);
    else
      second_branch->explore(this, search_data, candidates);
  }
}

/**
 * Traverse the kd-tree while discarding regions of space with hypersphere-hyperrectangle intersections.
 *
 * \param parent Parent node of the one being explored.
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D, typename S> template <typename C>
void kd_tree<T, D, S>::kd_node::intersect(const kd_node *parent, kd_search_data &search_data, C &candidates) const {

  // Intersection data is updated incrementally when the object is created, and restored when destroyed.
  kd_incremental incremental_update(this, parent, search_data);

  // Check if the hypersphere from current worst neighbour candidate intersects the region hyperrectangle.
  if (!(search_data.hyperrect_distance < search_data.farthest_distance))
    return;

  // Traverse left branch discarding regions of space.
  if (is_leaf & left_bit) {
    if (search_data.ignore_null_distances)
      left_leaf->intersect_ignoring_same(search_data, candidates);
    else
      left_leaf->intersect(search_data, candidates);
  } else {
    left_branch->intersect(this, search_data, candidates);
  }

  // Traverse right branch discarding regions of space.
  if (is_leaf & right_bit) {
    if (search_data.ignore_null_distances)
      right_leaf->intersect_ignoring_same(search_data, candidates);
    else
      right_leaf->intersect(search_data, candidates);
  } else {
    right_branch->intersect(this, search_data, candidates);
  }
}

/**
 * Process a leaf node that contains many buckets. Do not use any upper bounds in distance calculation (otherwise best candidates may be skipped).
 *
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D, typename S> template <typename C>
void kd_tree<T, D, S>::kd_leaf::explore(kd_search_data &search_data, C &candidates) const {

  if (search_data.ignore_null_distances) {

    // Process only the bucket elements different to p.
    for (unsigned int i=first_index; i < first_index + num_elements; ++i) {
      T distance = search_data.p.distance_to(search_data.data[i]);
      if (distance > (T) 0)
        candidates.push_back(kd_neighbour(i, search_data.p.distance_to(search_data.data[i])));
    }

  } else {
    // Process all the buckets in the node.
    for (unsigned int i=first_index; i < first_index + num_elements; ++i)
      // Create a new neighbour candidate with the point referenced by this node and push it into the K best ones.
      candidates.push_back(kd_neighbour(i, search_data.p.distance_to(search_data.data[i])));
  }

  // Update current farthest nearest neighbour distance.
  if (!candidates.empty())
    search_data.farthest_distance = candidates.front().squared_distance;
}

/**
 * Process a leaf node that contains many buckets without trying to discard them.
 *
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D, typename S> template <typename C>
void kd_tree<T, D, S>::kd_leaf::intersect(kd_search_data &search_data, C &candidates) const {

  // Process all the buckets in the node.
  for (unsigned int i=first_index; i < first_index + num_elements; ++i) {

    // Calculate the distance to the new candidate, upper bounded by the farthest nearest neighbour distance.
    T new_distance = search_data.p.distance_to(search_data.data[i], search_data.farthest_distance);

    // If less than the current farthest nearest neighbour then it's a valid candidate (equal is left for the all_in_range method).
    if (new_distance <= search_data.farthest_distance) {

      // Push it in the nearest neighbour container (will reject the previous farthest one).
      candidates.push_back(kd_neighbour(i, new_distance));

      // Update the distance to the new farthest nearest neighbour.
      search_data.farthest_distance = candidates.front().squared_distance;
    }
  }
}

/**
 * Process a leaf node that contains many buckets without trying to discard them.
 * Ignore any points with distance 0.
 *
 * \param search_data Auxiliar data structure used for tree traversal and incremental calculations.
 * \param candidates STL container-like object holding the current neighbour candidates.
 */
template <typename T, const unsigned int D, typename S> template <typename C>
void kd_tree<T, D, S>::kd_leaf::intersect_ignoring_same(kd_search_data &search_data, C &candidates) const {

  // Process all the buckets in the node.
  for (unsigned int i=first_index; i < first_index + num_elements; ++i) {

    // Calculate the distance to the new candidate, upper bounded by the farthest nearest neighbour distance.
    T new_distance = search_data.p.distance_to(search_data.data[i], search_data.farthest_distance);
    if (new_distance == (T) 0)
      continue;

    // If less than the current farthest nearest neighbour then it's a valid candidate (equal is left for the all_in_range method).
    if (new_distance <= search_data.farthest_distance) {

      // Push it in the nearest neighbour container (will reject the previous farthest one).
      candidates.push_back(kd_neighbour(i, new_distance));

      // Update the distance to the new farthest nearest neighbour.
      search_data.farthest_distance = candidates.front().squared_distance;
    }
  }
}


#ifdef DEBUG_KDTREE
template <typename T, const unsigned int D, typename S>
bool kd_tree<T, D, S>::verify() const {
  return root->verify(data, 0);
}

template <typename T, const unsigned int D, typename S>
bool kd_tree<T, D, S>::kd_node::verify(const kd_point* data, int axis) const {

  bool result;
  if (is_leaf & left_bit)
    result = left_leaf->verify_less_than(data, split_value, axis);
  else
    result = left_branch->verify_less_than(data, split_value, axis);

  if (!result)
    return false;

  if (is_leaf & right_bit)
    result = right_leaf->verify_more_than(data, split_value, axis);
  else
    result = right_branch->verify_more_than(data, split_value, axis);

  if (!result)
    return false;

  if (!(is_leaf & left_bit))
    result = left_branch->verify(data, (axis + 1) % D);

  if (!result)
    return false;

  if (!(is_leaf & right_bit))
    result = right_branch->verify(data, (axis + 1) % D);

  return result;
}

template <typename T, const unsigned int D, typename S>
bool kd_tree<T, D, S>::kd_node::verify_less_than(const kd_point* data, float value, int axis) const {

  bool result;
  if (is_leaf & left_bit)
    result = left_leaf->verify_less_than(data, value, axis);
  else
    result = left_branch->verify_less_than(data, value, axis);

  if (!result)
    return false;

  if (is_leaf & right_bit)
    result = right_leaf->verify_less_than(data, value, axis);
  else
    result = right_branch->verify_less_than(data, value, axis);

  return result;
}

template <typename T, const unsigned int D, typename S>
bool kd_tree<T, D, S>::kd_node::verify_more_than(const kd_point* data, float value, int axis) const {

  bool result;
  if (is_leaf & left_bit)
    result = left_leaf->verify_more_than(data, value, axis);
  else
    result = left_branch->verify_more_than(data, value, axis);

  if (!result)
    return false;

  if (is_leaf & right_bit)
    result = right_leaf->verify_more_than(data, value, axis);
  else
    result = right_branch->verify_more_than(data, value, axis);

  return result;
}

template <typename T, const unsigned int D, typename S>
bool kd_tree<T, D, S>::kd_leaf::verify_less_than(const kd_point*data, float value, int axis) const {

  for (unsigned int i=first_index; i < first_index + num_elements; ++i) {
    if (!(data[i][axis] <= value)) {
      fprintf(stderr, "Error: %.3f should be <= %.3f for axis %d\n", data[i][axis], value, axis);
      return false;
    }
  }

  return true;
}

template <typename T, const unsigned int D, typename S>
bool kd_tree<T, D, S>::kd_leaf::verify_more_than(const kd_point* data, float value, int axis) const {

  for (unsigned int i=first_index; i < first_index + num_elements; ++i) {
    if (!(data[i][axis] >= value)) {
      fprintf(stderr, "Error: %.3f should be >= %.3f for axis %d\n", data[i][axis], value, axis);
      return false;
    }
  }

  return true;
}
#endif // DEBUG_KDTREE
