/*
 * Quaternion.h
 *
 *  Created on: Dec 18, 2013
 *      Author: niber
 */

#ifndef QUATERNION_H_
#define QUATERNION_H_
#include <cmath>
#include <Eigen/Dense>
#include <ostream>
#include "typedefs.h"

class Quaternion {
public:
	Quaternion()
	{
		_q[1] = _q[2] = _q[3] = 0.0;
		_q[0] = 1.0;
	}

	Quaternion(scalar angle, const Vector & axis)
	{
		set_from_angle_axis(angle, axis);
	}

	void set_from_angle_axis(scalar angle, const Vector & axis)
	{
		scalar sin_a_half = std::sin(angle/2);
		_q[0] = std::cos(angle/2);
		_q[1] = sin_a_half*axis[0];
		_q[2] = sin_a_half*axis[1];
		_q[3] = sin_a_half*axis[2];
	}

	Quaternion(scalar q0, scalar q1, scalar q2, scalar q3)
	{
		_q[0] = q0;
		_q[1] = q1;
		_q[2] = q2;
		_q[3] = q3;
	}

	scalar & operator[](unsigned int i)
	{
		return _q[i];
	}

	scalar operator[](unsigned int i) const
	{
		return _q[i];
	}

	scalar norm_sqr() const
	{
		return _q[0]*_q[0] + _q[1]*_q[1] + _q[2]*_q[2] + _q[3]*_q[3];
	}

	scalar norm() const
	{
		return std::sqrt(norm_sqr());
	}

	Quaternion normalize() const
	{
		scalar nrm = norm();
		return Quaternion(_q[0]/nrm, _q[1]/nrm, _q[2]/nrm, _q[3]/nrm);
	}

	Quaternion conj() const
	{
		return Quaternion(_q[0], -_q[1], -_q[2], -_q[3]);
	}

	scalar dot(const Quaternion & q2) const {
		return _q[0]*q2[0] + _q[1]*q2[1] + _q[2]*q2[2] + _q[3]*q2[3];
	}

	void set_to_unity()
	{
		_q[1] = _q[2] = _q[3] = 0.0;
		_q[0] = 1.0;
	}

	void get_angle_axis(scalar & angle, Vector & axis) const
	{
		scalar vec_norm = std::sqrt(_q[1]*_q[1] + _q[2]*_q[2] + _q[3]*_q[3]);
		angle = 2. * std::atan2(vec_norm, _q[0]);
		if(vec_norm < 1e-6)
			axis = Vector::Zero();
		else {
			axis[0] = _q[1] / vec_norm;
			axis[1] = _q[2] / vec_norm;
			axis[2] = _q[3] / vec_norm;
		}
	}

	void apply_rotation(const Vector & vec, Vector & ret) const
	{
		ret[0]= (1 - 2*(_q[2]*_q[2] + _q[3]*_q[3]))*vec[0] +
					  2*(_q[1]*_q[2] - _q[0]*_q[3])*vec[1] +
					  2*(_q[0]*_q[2] + _q[1]*_q[3])*vec[2];
		ret[1] = 2*(_q[1]*_q[2] + _q[0]*_q[3])*vec[0] +
					  (1 - 2*(_q[1]*_q[1] + _q[3]*_q[3]))*vec[1] +
					  2*(_q[2]*_q[3] - _q[0]*_q[1])*vec[2];
		ret[2] = 2*(_q[1]*_q[3] - _q[0]*_q[2])*vec[0] +
					  2*(_q[0]*_q[1] + _q[2]*_q[3])*vec[1] +
					  (1 - 2*(_q[1]*_q[1] + _q[2]*_q[2]))*vec[2];
	}

	Vector apply_rotation(const Vector & vec) const
	{
		Vector ret;
		apply_rotation(vec, ret);
		return ret;
	}

	void apply_inv_rotation(const Vector & vec, Vector & ret) const
	{
		ret[0] = (1 - 2*(_q[2]*_q[2] + _q[3]*_q[3]))*vec[0] +
					  2*(_q[1]*_q[2] + _q[0]*_q[3])*vec[1] +
					  2*(_q[1]*_q[3] - _q[0]*_q[2])*vec[2];
		ret[1] = 2*(_q[1]*_q[2] - _q[0]*_q[3])*vec[0] +
					  (1 - 2*(_q[1]*_q[1] + _q[3]*_q[3]))*vec[1] +
					  2*(_q[0]*_q[1] + _q[2]*_q[3])*vec[2];
		ret[2] = 2*(_q[0]*_q[2] + _q[1]*_q[3])*vec[0] +
					  2*(_q[2]*_q[3] - _q[0]*_q[1])*vec[1] +
					  (1 - 2*(_q[1]*_q[1] + _q[2]*_q[2]))*vec[2];
	}

	Vector apply_inv_rotation(const Vector & vec) const
	{
		Vector ret;
		apply_inv_rotation(vec, ret);
		return ret;
	}

	void to_rot_matrix(Matrix & ret) const
	{
		ret(0, 0) = 1 - 2*(_q[2]*_q[2] + _q[3]*_q[3]);
		ret(0, 1) = 2*(_q[1]*_q[2] - _q[0]*_q[3]);
		ret(0, 2) = 2*(_q[0]*_q[2] + _q[1]*_q[3]);
		ret(1, 0) = 2*(_q[1]*_q[2] + _q[0]*_q[3]);
		ret(1, 1) = 1 - 2*(_q[1]*_q[1] + _q[3]*_q[3]);
		ret(1, 2) = 2*(_q[2]*_q[3] - _q[0]*_q[1]);
		ret(2, 0) = 2*(_q[1]*_q[3] - _q[0]*_q[2]);
		ret(2, 1) = 2*(_q[0]*_q[1] + _q[2]*_q[3]);
		ret(2, 2) = 1 - 2*(_q[1]*_q[1] + _q[2]*_q[2]);
	}

	Matrix to_rot_matrix() const
	{
		Matrix ret;
		to_rot_matrix(ret);
		return ret;
	}

private:
	scalar _q[4];
};

inline Quaternion operator*(const Quaternion & p, const Quaternion & q)
{
	return Quaternion(
				p[0]*q[0]-(p[1]*q[1]+p[2]*q[2]+p[3]*q[3]),
				p[0]*q[1] + q[0]*p[1] + p[2]*q[3] - p[3]*q[2],
				p[0]*q[2] + q[0]*p[2] + p[3]*q[1] - p[1]*q[3],
				p[0]*q[3] + q[0]*p[3] + p[1]*q[2] - p[2]*q[1]
		);
}

inline Quaternion operator*(const Quaternion & p, scalar s)
{
	return Quaternion(s*p[0], s*p[1], s*p[2], s*p[3]);
}

inline Quaternion operator*(scalar s, const Quaternion & p)
{
	return Quaternion(s*p[0], s*p[1], s*p[2], s*p[3]);
}

inline Quaternion operator+(const Quaternion & p, const Quaternion & q)
{
	return Quaternion(
				p[0]+q[0],
				p[1]+q[1],
				p[2]+q[2],
				p[3]+q[3]
		);
}

inline Quaternion operator-(const Quaternion & p, const Quaternion & q)
{
	return Quaternion(
				p[0]-q[0],
				p[1]-q[1],
				p[2]-q[2],
				p[3]-q[3]
		);
}

inline std::ostream & operator<<(std::ostream & ss, const Quaternion & q)
{
	ss << "(" << q[0] << ", [" << q[1] << ", " << q[2] << ", " << q[3] << "])";
	return ss;
}

#endif /* QUATERNION_H_ */
