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
 * \mainpage Kche-tree Template Documentation
 *
 * \section introduction Introduction
 * Kche-tree is a template for generic cache-aware and non-mutable kd-trees.
 * Its main purpose is to provide an easy to use but powerful implementation
 * of the typical kd-tree structure functionality with very low latencies.
 *
 * It provides the following basic operations:
 * - \link kd_tree::build Build\endlink: create a kd-tree from a training set of
 *   feature vectors. Median splitting is used to keep the tree balanced. Cost: O(n log² n).
 * - \link kd_tree::knn K nearest neighbours\endlink: retrieve the K nearest
 *   neighbours of a given feature vector. Estimated average cost: O(log K log n).
 * - \link kd_tree::all_in_range All neighbours within a range\endlink: retrieve all the
 *   neighbours inside a maximum distance radius from a given feature vector.
 *   Estimated average cost: O(log m log n) with \e m the number of neighbours in the range.
 *
 * The template has been designed to minimize the number of cache misses combined
 * with many algorithmic techniques and ideas.\n
 * Here are some of its features:
 * - Incremental calculation of the hyperrectangle-hypersphere intersections.
 * - Internal data permutation to increase cache hits.
 * - Contiguous bucket data to reduce the leaf node size.
 * - Preorder memory allocation during the build to increase cache hits when traversing.
 * - Exploration/intersection recursive scheme to reduce the number of calculations performed.
 * - Use of specific k-neighbours optimized containers: k-vectors and k-heaps.
 * - Distance calculations with upper bounds allowing early returns.
 * - Designed to easily enable SSE optimizations by template specialization. Example and benchmarks included.
 * - Binary file format and stream operators provide to easily save and load the kd-trees.
 *
 * The current version is not still thread-safe. This is expected to be solved in future releases
 * along with OpenMP optimizations.
 *
 * Additionally, the following tools are provided:
 * - Example tool: shows a quite simple use of the template. Can be found in file example.cpp.
 * - Testing tool: tests the correction of the results and the I/O functions of the kd-tree.
 *   Comes in various flavors: random feature vector generation or file input and SSE optimizations.
 *   Can be found in file test_kdtree.cpp.
 * - Benchmark tool: measures the time spent in building the kd-tree and finding the K nearest neighbours.
 *   Like the testing tool comes in various flavors allowing to test its speed with random or
 *   preprocessed data from a file. Can be found in file speed_kdtree.cpp.
 *
 *
 * \section installation Usage
 * Kche-trees are quite simple to use. Just include the \c kd-tree.h file
 * and proceed like in the usage example \c example.cpp. Since it's a template
 * no additional files required to be compiled or linked. However it requires
 * the following files to be present where the main header is:
 * - kd-tree template files: kd-tree.h, kd-tree.cpp, kd-tree_io.cpp
 * - feature vector files: feature_vector.h, feature_vector.cpp
 * - k-neighbour container files: k-vector.h, k-vector.cpp for k-vectors,
 *   k-heap.h, k-heap.cpp, indirect_heap.h, indirect_heap.cpp for k-heaps.
 * - <b>LICENSE</b>: don't forget this is GPL code. Yours must be also if you use this.
 *
 * In case of using the floating point/24 dimension SSE specialization,
 * the file kd-tree_sse_24d.h and the compilation flag \c -msse will be required.
*/

/**
 * \file kd-tree.h
 * \brief Template for generic kd-trees.
 * \author Leandro Graciá Gil
*/

#ifndef _KD_TREE_H_
#define _KD_TREE_H_

// Include STL vectors for K neighbours output.
#include <vector>

// STL streams.
#include <iostream>

// Include k-neighbours containers (k-heaps and k-vectors).
#include "k-heap.h"
#include "k-vector.h"

// Include template for feature vectors.
#include "feature_vector.h"


/**
 * \brief Template for generic cache-aware kd-trees of any type.
 *
 * Element insertions and deletions are not currently supported in behalf of a design based on cache efficiency.
 * \warning This class is currently not thread-safe. This feature will be added in future releases.
 *
 * \tparam T Data type of the elements in the kd-tree. Requires copy, internal + += = *, external (* int) and comparison < <= > operators.
 * \tparam D Number of dimensions in the input data.
 * \tparam S (Optional) K-best elements container. Responds to empty, size, back, push_back and pop_front.
 *  Defaults to k_vector<vector_distance<T>, vector_distance<T> >, but k_heap<vector_distance<T>, vector_distance<T> > is also valid.
 */
template <typename T, const unsigned int D, typename S = k_vector<vector_distance<T>, vector_distance<T> > >
class kd_tree {
public:
  /// Consider compatible feature vectors as D-dimensional points in the space.
  typedef feature_vector<T, D> kd_point;

  /// Define the type used for kd-tree neighbours.
  typedef vector_distance<T> kd_neighbour;

  // Constructors and destructors.
  kd_tree(); ///< Default constructor. Creates an empty and uninitialized kd-tree.
  ~kd_tree(); ///< Default destructor.

  // Basic kd-tree operations.
  bool build(const kd_point *points, unsigned int num_points, unsigned int bucket_size = 32); ///< Build a kd-tree from a set of input points. Cost: O(n log² n).
  void knn(const kd_point &p, unsigned int K, std::vector<kd_neighbour> &output, T epsilon = (T) 0, bool ignore_p_in_tree = false) const; ///< Get the K nearest neighbours of a point. Estimated average cost: O(log K log n).
  void all_in_range(const kd_point &p, T distance, std::vector<kd_neighbour> &output, bool ignore_p_in_tree = false) const; ///< Get all neighbours within a distance from a point. Estimated average Cost: O(log m log n) depending on the number of results m.

  // Subscript operator for accesing stored data (will fail on non-built kd-trees).
  const kd_point & operator [] (unsigned int index) const;

  // Stream operators.
  template <typename T_, const unsigned int D_, typename S_>
  friend std::istream & operator >> (std::istream &in, kd_tree<T_, D_, S_> &kdtree); ///< Input operator (read from stream). Throws an exception in case of error.

  template <typename T_, const unsigned int D_, typename S_>
  friend std::ostream & operator << (std::ostream &out, const kd_tree<T_, D_, S_> &kdtree); ///< Output operator (save to stream).

  // Kd-tree properties.
  unsigned int get_D() const { return D; } ///< Get the number of dimensions of the input data.
  unsigned int get_N() const { return num_elements; } ///< Get the number of elements stored in the tree.

#ifdef DEBUG_KDTREE
  bool verify() const;
#endif

protected:
  /// Structure holding data for incremental hyperrectangle-hypersphere intersection and nearest neighbour search.
  struct kd_search_data {

    const kd_point &p; ///< Reference input point.
    const kd_point *data; ///< Pointer to permutated data array.
    unsigned int K; ///< Number of neighbours to retrieve.

    T hyperrect_distance; ///< Distance to the current nearest point in the hyperrectangle.
    T farthest_distance; ///< Current distance from the farthest nearest neighbour to the reference point.
    bool ignore_null_distances;  ///< Used to exclude the source point if it's already in the tree.

    /// Axis-ordered point and hyperrectangle structure. Used internally to increase the cache hits.
    struct axis_data {
      T p; ///< Per-axis reference input point.
      T nearest; ///< Per-axis nearest point in the current hyperrectangle.
    } axis[D]; ///< Per-axis data defined this way to reduce cache misses.

    /// Initialize data for a tree search with incremental intersection calculation.
    kd_search_data(const kd_point &p, const kd_point *data, unsigned int K, bool ignore_p_in_tree);
  };

  /// Kd-tree leaf node.
  struct kd_leaf {
    unsigned int first_index; ///< Index of the first element contained by the leaf node.
    unsigned int num_elements; ///< Number of elements contained by the node.

    /// Leaf constructor.
    kd_leaf(unsigned int first_index, unsigned int num_elements) :
      first_index(first_index), num_elements(num_elements) {}

    /// Construct from an input stream.
    kd_leaf(std::istream &input);

    /// Process a leaf node with many buckets on it. Do not use any upper bounds on distance.
    template <typename C> void explore(kd_search_data &data, C &candidates) const;

    /// Process a leaf node with many buckets on it. Allows partial distance calculations.
    template <typename C> void intersect(kd_search_data &data, C &candidates) const;
    template <typename C> void intersect_ignoring_same(kd_search_data &data, C &candidates) const;

    /// Write to stream.
    bool write_to_binary_stream(std::ostream &out);

#ifdef DEBUG_KDTREE
  bool verify_less_than(const kd_point* data, float value, int axis) const;
  bool verify_more_than(const kd_point* data, float value, int axis) const;
#endif
  };

  /// Kd-tree branch node.
  struct kd_node {

    union {
      kd_node *left_branch; ///< Left branch.
      kd_leaf *left_leaf; ///< Left leaf.
    };

    union {
      kd_node *right_branch; ///< Right branch.
      kd_leaf *right_leaf; ///< Right leaf.
    };

    T split_value; ///< Value used to split the hyperspace in two.

    // The leaf flags use the two uppermost bits of the axis index. Won't handle more than 2^30 dimensions.
    union {
      unsigned int axis; ///< Index of the current axis being split.
      unsigned int is_leaf; ///< Bitmask used to check if left and right nodes are leafs or branches.
    };

    // Bit masks to access the leaf and axis information.
    static const unsigned int left_bit  = 0x80000000U; ///< Mask used to access the left branch bit in is_leaf.
    static const unsigned int right_bit = 0x40000000U; ///< Mask used to access the right branch bit in is_leaf.
    static const unsigned int axis_mask = 0x3FFFFFFFU; ///< Mask used to access the axis bits.

    // Constructors and destructor.
    kd_node() : left_branch(NULL), right_branch(NULL), is_leaf(0) {} ///< Default constructor.
    kd_node(std::istream &input); ///< Construct from an input stream.
    ~kd_node(); ///< Default destructor.

#ifdef DEBUG_KDTREE
  bool verify(const kd_point* data, int axis) const;
  bool verify_less_than(const kd_point* data, float value, int axis) const;
  bool verify_more_than(const kd_point* data, float value, int axis) const;
#endif

    /// Per axis element comparison functor. Used to apply STL sorting algorithms to individual axes.
    struct kd_axis_comparer {
      const kd_point *data; ///< Array of input data.
      unsigned int axis; ///< Current axis used for sorting.

      /// Axis-th element comparison. Used to perform per-dimension data sorting.
      bool operator () (const unsigned int &i1, const unsigned int &i2) const {
        return data[i1][axis] < data[i2][axis];
      }
    };

    // --- Training-related --- //

    /// Build the kd-tree recursively.
    static kd_node *build(const kd_point *data, unsigned int *index, unsigned int n,
        kd_node *parent, unsigned int bucket_size, unsigned int &processed);

    /// Find a pivot to split the space in two by a chosen dimension during training.
    unsigned int split(unsigned int *index, unsigned int n, const kd_axis_comparer &comparer);

    // --- Search-related --- //

    /// Traverse the kd-tree looking for nearest neighbours candidates based on Manhattan distances.
    template <typename C> void explore(const kd_node *parent, kd_search_data &data, C &candidates) const;

    /// Traverse the kd-tree checking hypersphere-hyperrectangle intersections to discard regions of the space.
    template <typename C> void intersect(const kd_node *parent, kd_search_data &data, C &candidates) const;

    // --- IO-related --- //

    /// Write to stream.
    bool write_to_binary_stream(std::ostream &out);
  };

  /// Incremental hyperrectangle-hypersphere intersection calculator. Designed so that the object lifespan handles increments.
  class kd_incremental {
  public:
    /// Create an temporary incremental calculator object. Will update the current data according with the selected branch.
    kd_incremental(const kd_node *node, const kd_node *parent, kd_search_data &data);

    /// Destroy a temporary incremental calculator object. Will restore any previous modifications of the incremental data.
    ~kd_incremental();

  protected:
    kd_search_data &search_data; ///< Reference to the search data being used.
    unsigned int parent_axis; ///< Axis that defines the hyperspace splitting.

    bool modified; ///< Flag indicating if the values were modified as part of the incremental update.
    T previous_axis_nearest; ///< Previous value of the local axis in the hyperrectangle.
    T previous_hyperrect_distance; ///< Previous value of the distance to the nearest point in the hyperrectangle.
  };

  // Internal methods.
  void release(); ///< Release memory allocated by the kd-tree including tree nodes.

  // Kd-tree data.
  kd_node *root; ///< Root node of the tree. Will be \c NULL in empty trees.
  kd_point *data; ///< Data of the kd-tree. Copied with element permutations during the tree building.
  unsigned int *permutation; ///< Permutation indices applied to kd-tree data to enhance cache behaviour.
  unsigned int *inverse_perm; ///< Inverse of the permutation applied to indices in the permutation array.
  unsigned int num_elements; ///< Number of elements currently in the tree.

  // File serialization settings.
  static const char *file_header; ///< Header written when serializing kd-trees.
  static const unsigned short file_header_length; ///< Length in bytes of the header.
  static const unsigned short file_version[2]; ///< Tuple of major and minor version of the current kd-tree file format.
  static const unsigned short signature; ///< Signature value used to check the end of file according to the format.
};

// Template implementation.
#include "kd-tree.cpp"
#include "kd-tree_io.cpp"

#endif
