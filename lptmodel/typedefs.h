#ifndef TYPEDEFS_H_
#define TYPEDEFS_H_
#include <Eigen/Core>
#include "json.hpp"

// scalar type
using scalar = float;

// 3D vector type
using Vector = Eigen::Matrix<scalar, 3, 1>;

// Matrix
using Matrix = Eigen::Matrix<scalar, 3, 3>;

// Convenience typedef
using json = nlohmann::json;

// Implement json conversion for Vector
namespace Eigen {
inline void to_json(json & j, const Matrix<scalar, 3, 1> & v)
{
	j = json::array({v[0], v[1], v[2]});
}

inline void from_json(const json & j, Matrix<scalar, 3, 1> & v)
{
	if( ! j.is_array())
		throw std::runtime_error("Expected array");
	if( ! j.size() == 3)
		throw std::runtime_error("Invalid array size, expected 3 elements");

	for(int i = 0; i < 3; ++i)
		v[i] = j.at(i).get<scalar>();
}


}

#endif /* TYPEDEFS_H_ */
