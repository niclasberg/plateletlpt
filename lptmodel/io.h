#ifndef IO_H_
#define IO_H_
#include <ostream>
#include <istream>
#include <sstream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <Eigen/Dense>
#include "typedefs.h"

// Binary input and output
template<class T>
inline void read_from_stream(std::istream & in, T * v, int n)
{
	in.read(reinterpret_cast<char *>(v), n*sizeof(T));
}

template<class T>
inline void read_from_stream(std::istream & in, T & v)
{
	read_from_stream(in, &v, 1);
}

template<class T>
inline void write_to_stream(std::ostream & out, const T * v, int n)
{
	out.write(reinterpret_cast<const char *>(v), n*sizeof(T));
}

template<class T>
inline void write_to_stream(std::ostream & out, const T & v)
{
	write_to_stream(out, &v, 1);
}

inline void read_from_stream(std::istream & in, Vector & vector)
{
	read_from_stream(in, vector.data(), 3);
}

inline void write_to_stream(std::ostream & out, const Vector & vector)
{
	write_to_stream(out, vector.data(), 3);
}

inline void read_from_stream(std::istream & in, Matrix & matrix)
{
	read_from_stream(in, matrix.data(), 9);
}

inline void write_to_stream(std::ostream & out, const Matrix & matrix)
{
	write_to_stream(out, matrix.data(), 9);
}

template<class T, int NR, int NC>
inline void read_from_stream(std::istream & in, Eigen::Matrix<T, NR, NC> & eigenMatrix)
{
	using MatrixType = typename Eigen::Matrix<T, NR, NC>;
	using IndexType = typename MatrixType::Index;

	// Read matrix dimensions
	IndexType rows = 0, cols = 0;
	read_from_stream(in, rows);
	read_from_stream(in, cols);
	
	// Allocate space and read
	eigenMatrix.resize(rows, cols);
	read_from_stream(in, eigenMatrix.data(), rows*cols);
}

template<class T, int NR, int NC>
inline void write_to_stream(std::ostream & out, const Eigen::Matrix<T, NR, NC> & eigenMatrix)
{
	using MatrixType = typename Eigen::Matrix<T, NR, NC>;
	using IndexType = typename MatrixType::Index;

	// Write dimensions
	IndexType rows = eigenMatrix.rows(), cols = eigenMatrix.cols();
	write_to_stream(out, rows);
	write_to_stream(out, cols);

	// Write data
	write_to_stream(out, eigenMatrix.data(), rows*cols);
}

// JSON
template<class T>
inline T jsonGetOrDefault(const json & jsonObject, const std::string & key, const T & defaultValue)
{
	if(jsonObject.count(key))
		return jsonObject.at(key).get<T>();
	return defaultValue;
}

namespace detail {
// Base case
inline void stringify_impl(std::stringstream & ss) { }

template<class Thead, class ... Trest>
inline void stringify_impl(std::stringstream & ss, Thead & head, Trest & ... rest)
{
	ss << head;
	stringify_impl(ss, rest...);
}

}

template<class ... Args>
inline std::string stringify(Args & ... args)
{
	std::stringstream ss;
	detail::stringify_impl(ss, args...);
	return ss.str();
}

// Makedir (thread safe)
namespace detail {
inline int do_mkdir(char * path, mode_t mode)
{
	struct stat st;
	int status = 0;

	if (stat(path, &st) != 0) {
		if (::mkdir(path, mode) != 0 && errno != EEXIST)
			status = -1;
	} else if (!S_ISDIR(st.st_mode)) {
		errno = ENOTDIR;
		status = -1;
	}

	return status;
}
}

inline void mkdir(const char * folder)
{
	/* Create output folder structure */
	std::string folder_tmp(folder);
	char * foldercopy = const_cast<char *>(folder_tmp.c_str());
	char * start, * end;

	int status = 0;
	start = foldercopy;

	while(status == 0 && (end = strchr(start, '/')) != 0) {
		if(start != end) {
			*end = '\0';
			status = detail::do_mkdir(foldercopy, 0777);
			*end = '/';
		}

		start = end + 1;
	}

	if(status == 0)
		detail::do_mkdir(foldercopy, 0777);
}

#endif /* IO_H_ */
