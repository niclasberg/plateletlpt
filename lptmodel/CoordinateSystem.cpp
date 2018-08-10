#include "CoordinateSystem.h"
#include "Quaternion.h"
#include <iostream>
#include "typedefs.h"

void CoordinateSystem::fromJSON(const json & j)
{
	if(j.count("translate"))
		offset() = j.at("translate").get<Vector>();
	else
		offset() = Vector(0, 0, 0);

	Vector axis(0, 0, 1);
	scalar angle = 0;
	scalar rpm = 0;

	if(j.count("rotate")) {
		const json & jRotate = j.at("rotate");

		if(jRotate.count("axis"))
			axis = jRotate.at("axis").get<Vector>();
		if(jRotate.count("angle"))
			angle = jRotate.at("angle").get<scalar>();
		if(jRotate.count("rpm"))
			rpm = jRotate.at("rpm").get<scalar>();
	}

	axis /= axis.norm();
	orientation().set_from_angle_axis(angle * M_PI / 180., axis);
	angularVelocity() = 2. * M_PI * (rpm / 60.) * axis;
	t0() = 0.;
}

Quaternion CoordinateSystem::getOrientation(scalar t) const
{
	const scalar om_norm = angularVelocity_.norm();
	const scalar rot_angle = (t - t0_) * om_norm;
	if(std::abs(om_norm) > std::numeric_limits<scalar>::epsilon()) // avoid singularity when the norm of the angular velocity is small
		return (Quaternion(rot_angle, angularVelocity_ / om_norm) * orientation_);
	return orientation_;
}

void CoordinateSystem::normalVectorToWorldFrame(scalar t, const Vector & normal, Vector & ret) const
{
	getOrientation(t).apply_rotation(normal, ret);
}

void CoordinateSystem::normalVectorToLocalFrame(scalar t, const Vector & normal, Vector & ret) const
{
	getOrientation(t).apply_inv_rotation(normal, ret);
}

void CoordinateSystem::velocityToWorldFrame(scalar t, const Vector & pos, const Vector & velocity, Vector & ret) const
{
	getOrientation(t).apply_rotation(velocity, ret);
	ret += angularVelocity_.cross(pos - offset_);
}

void CoordinateSystem::velocityToLocalFrame(scalar t, const Vector & pos, const Vector & velocity, Vector & ret) const
{
	getOrientation(t).apply_inv_rotation(velocity - (angularVelocity_.cross(pos - offset_)), ret);
}


void CoordinateSystem::positionToWorldFrame(scalar t, const Vector & pos, Vector & ret) const
{
	getOrientation(t).apply_rotation(pos, ret);
	ret += offset_;
}

void CoordinateSystem::positionToLocalFrame(scalar t, const Vector & pos, Vector & ret) const
{
	ret = pos - offset_;
	getOrientation(t).apply_inv_rotation(pos, ret);
}		

