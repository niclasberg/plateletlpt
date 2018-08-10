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
 * \file kd-tree_io.cpp
 * \brief Template implementations for stream operations in kd-trees.
 * \author Leandro Graciá Gil
 */

// Includes from STL and C standard library.
#include <cstdio>
#include <stdexcept>
#include <typeinfo>

// File serialization settings.
template <typename T, const unsigned int D, typename S> const char *kd_tree<T, D, S>::file_header = "kdtree";
template <typename T, const unsigned int D, typename S> const unsigned short kd_tree<T, D, S>::file_header_length = 6;
template <typename T, const unsigned int D, typename S> const unsigned short kd_tree<T, D, S>::file_version[2] = { 1, 0 };
template <typename T, const unsigned int D, typename S> const unsigned short kd_tree<T, D, S>::signature = 0xCAFE;

/**
 * Standard input stream operator.
 * Loads the kd-tree from a stream in binary format.
 *
 * \exception std::runtime_error A runtime error exception may be thrown in case of error reading or processing the kd-tree.
 *
 * \param in Input stream.
 * \param kdtree Kd-tree where results will be saved.
 * \return Input stream after reading the data.
 */
template <typename T, const unsigned int D, typename S>
std::istream & operator >> (std::istream &in, kd_tree<T, D, S> &kdtree) {

  // Read header.
  char header[kdtree.file_header_length + 1];
  in.read(header, kdtree.file_header_length);
  header[kdtree.file_header_length] = '\0';
  if (!in.good())
    throw std::runtime_error("error reading header data");

  // Check header.
  if (strcmp(header, kdtree.file_header))
    throw std::runtime_error("invalid kd-tree header");

  // Read file version.
  unsigned short version[2];
  in.read(reinterpret_cast<char *>(version), 2 * sizeof(unsigned short));
  if (!in.good())
    throw std::runtime_error("error reading version data");

  // Check supported file versions.
  if (version[0] != 1 || version[1] != 0)
    throw std::runtime_error("unsupported kd-tree file version");

  // Read type name length.
  unsigned short name_length;
  in.read(reinterpret_cast<char *>(&name_length), sizeof(unsigned short));
  if (!in.good())
    throw std::runtime_error("error reading type length data");

  // Read type name.
  char type_name[name_length + 1];
  in.read(type_name, name_length);
  if (!in.good())
    throw std::runtime_error("error reading type data");
  type_name[name_length] = '\0';

  // Check type name.
  if (strcmp(typeid(T).name(), type_name)) {
    char error_msg[name_length + strlen(typeid(T).name()) + 50];
    sprintf(error_msg, "kd-tree type doesn't match: found %s, expected %s", type_name, typeid(T).name());
    throw std::runtime_error(error_msg);
  }

  // Read number of dimensions.
  unsigned int num_dimensions;
  in.read(reinterpret_cast<char *>(&num_dimensions), sizeof(unsigned int));
  if (!in.good())
    throw std::runtime_error("error reading dimensionality data");

  // Check number of dimensions.
  if (num_dimensions != D) {
    char error_msg[75];
    sprintf(error_msg, "number of dimensions doesn't match: found %d, expected %d", num_dimensions, D);
    throw std::runtime_error(error_msg);
  }

  // Release any previous kd-tree (file seems to be compatible).
  kdtree.release();

  // Define aliases for local kd-tree types.
  typedef typename kd_tree<T, D, S>::kd_point point_type;
  typedef typename kd_tree<T, D, S>::kd_node node_type;
  typedef typename kd_tree<T, D, S>::kd_leaf leaf_type;

  // Read and check number of elements.
  in.read(reinterpret_cast<char *>(&kdtree.num_elements), sizeof(unsigned int));
  if (!in.good())
    throw std::runtime_error("error reading number of elements");
  if (kdtree.num_elements == 0)
    throw std::runtime_error("invalid number of elements in the tree");

  // Allocate and read the permutation array.
  kdtree.permutation = new unsigned int [kdtree.num_elements];
  in.read(reinterpret_cast<char *>(kdtree.permutation), kdtree.num_elements * sizeof(unsigned int));
  if (!in.good())
    throw std::runtime_error("error reading kd-tree data permutation");

  // Build the inverse permutation array.
  kdtree.inverse_perm = new unsigned int [kdtree.num_elements];
  for (unsigned int i=0; i<kdtree.num_elements; ++i)
    kdtree.inverse_perm[kdtree.permutation[i]] = i;

  // Allocate and read kd-tree data array.
  kdtree.data = new point_type [kdtree.num_elements];
  in.read(reinterpret_cast<char *>(kdtree.data), kdtree.num_elements * sizeof(point_type));
  if (!in.good())
    throw std::runtime_error("error reading kd-tree data");

  // Read the tree structure from the stream.
  kdtree.root = new node_type(in);

  // Read the signature value.
  unsigned short signature;
  in.read(reinterpret_cast<char *>(&signature), sizeof(unsigned short));
  if (in.fail() || signature != kdtree.signature)
    throw std::runtime_error("error reading kd-tree signature");

  // Return the input stream.
  return in;
}

/**
 * Standard output stream operator.
 * Saves the kd-tree into a stream in binary format.
 *
 * \param out Output stream.
 * \param kdtree Kd-tree being saved.
 * \return Output stream after writting the data.
 */
template <typename T, const unsigned int D, typename S>
std::ostream & operator << (std::ostream &out, const kd_tree<T, D, S> &kdtree) {

  // Define aliases for local kd-tree types.
  typedef typename kd_tree<T, D, S>::kd_point point_type;
  typedef typename kd_tree<T, D, S>::kd_node node_type;

  // Write header.
  out.write(kdtree.file_header, kdtree.file_header_length);

  // Write file version.
  out.write(reinterpret_cast<const char *>(kdtree.file_version), 2 * sizeof(unsigned short));

  // Get information about the template type T.
  unsigned short name_length = strlen(typeid(T).name());

  // Write name of the type T with its length.
  out.write(reinterpret_cast<const char *>(&name_length), sizeof(unsigned short));
  out.write(typeid(T).name(), name_length);

  // Write number of dimensions of the data.
  const unsigned int D_ = D;
  out.write(reinterpret_cast<const char *>(&D_), sizeof(unsigned int));

  // Write number of elements in the tree.
  out.write(reinterpret_cast<const char *>(&kdtree.num_elements), sizeof(unsigned int));

  // Write tree data.
  out.write(reinterpret_cast<const char *>(kdtree.permutation), kdtree.num_elements * sizeof(unsigned int));

  // Write tree data permutation.
  out.write(reinterpret_cast<const char *>(kdtree.data), kdtree.num_elements * sizeof(point_type));

  // Write the kd-tree structure recursively.
  kdtree.root->write_to_binary_stream(out);

  // Write a 2-byte signature at the end: simplifies error checking.
  out.write(reinterpret_cast<const char *>(&kdtree.signature), sizeof(unsigned short));

  // Return the output stream.
  return out;
}

/**
 * Build a kd-tree branch node from the data in a binary input stream.
 * \warning A std::runtime_error exception may be thrown in case of error reading the data.
 *
 * \param in Input stream.
 */
template <typename T, const unsigned int D, typename S>
kd_tree<T, D, S>::kd_node::kd_node(std::istream &in) {

  // Allocate node.
  typedef typename kd_tree<T, D, S>::kd_node node_type;
  typedef typename kd_tree<T, D, S>::kd_leaf leaf_type;

  // Read node data.
  in.read(reinterpret_cast<char *>(&split_value), sizeof(T));
  in.read(reinterpret_cast<char *>(&is_leaf), sizeof(unsigned int));
  if (!in.good())
    throw std::runtime_error("error reading node data");

  // Process the left branch or leaf.
  if (is_leaf & left_bit)
    left_leaf = new leaf_type(in);
  else
    left_branch = new node_type(in);

  // Process the right branch or leaf.
  if (is_leaf & right_bit)
    right_leaf = new leaf_type(in);
  else
    right_branch = new node_type(in);
}

/**
 * Build a kd-tree branch node from the data in a binary input stream.
 * \warning A std::runtime_error exception may be thrown in case of error reading the data.
 *
 * \param in Input stream.
 */
template <typename T, const unsigned int D, typename S>
kd_tree<T, D, S>::kd_leaf::kd_leaf(std::istream &in) {

  // Read leaf node data.
  in.read(reinterpret_cast<char *>(&first_index),  sizeof(unsigned int));
  in.read(reinterpret_cast<char *>(&num_elements), sizeof(unsigned int));
  if (!in.good())
    throw std::runtime_error("error reading leaf node data");
}

/**
 * Write a kd-tree branch node to a binary ouput stream.
 *
 * \param out Output stream.
 * \return \c true if successful, \c false in case of write error.
 */
template <typename T, const unsigned int D, typename S>
bool kd_tree<T, D, S>::kd_node::write_to_binary_stream(std::ostream &out) {

  // Write split value and node axis/leaf information.
  out.write(reinterpret_cast<const char *>(&split_value), sizeof(T));
  out.write(reinterpret_cast<const char *>(&is_leaf), sizeof(unsigned int));
  if (!out.good())
    return false;

  // Process the left branch or leaf.
  bool retval;
  if (is_leaf & left_bit)
    retval = left_leaf->write_to_binary_stream(out);
  else
    retval = left_branch->write_to_binary_stream(out);
  if (!retval)
    return false;

  // Process the right branch or leaf.
  if (is_leaf & right_bit)
    retval = right_leaf->write_to_binary_stream(out);
  else
    retval = right_branch->write_to_binary_stream(out);
  if (!retval)
    return false;

  return true;
}

/**
 * Write a kd-tree leaf node to a binary ouput stream.
 *
 * \param out Output stream.
 * \return \c true if successful, \c false in case of write error.
 */
template <typename T, const unsigned int D, typename S>
bool kd_tree<T, D, S>::kd_leaf::write_to_binary_stream(std::ostream &out) {

  // Write left leaf node information.
  out.write(reinterpret_cast<const char *>(&first_index), sizeof(unsigned int));
  out.write(reinterpret_cast<const char *>(&num_elements), sizeof(unsigned int));

  // Check stream status.
  if (!out.good())
    return false;
  return true;
}
