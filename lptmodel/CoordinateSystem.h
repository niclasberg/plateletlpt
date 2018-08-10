#ifndef COORDINATESYSTEM_H_
#define COORDINATESYSTEM_H_
#include <Eigen/Dense>
#include "Quaternion.h"
#include <iostream>
#include "typedefs.h"
#include "macros.h"

class CoordinateSystem {
public:
	GETSET(Vector, offset)
	GETSET(Quaternion, orientation)
	GETSET(scalar, t0)
	GETSET(Vector, angularVelocity)

	void fromJSON(const json &);

	Quaternion getOrientation(scalar t) const;
	void normalVectorToWorldFrame(scalar t, const Vector & normal, Vector & ret) const;
	void normalVectorToLocalFrame(scalar t, const Vector & normal, Vector & ret) const;

	/*
	 * t: time
	 * pos: position in world frame
	 * velocity: velocity in local frame
	 * ret: velocity in world frame
	 */
	void velocityToWorldFrame(scalar t, const Vector & pos, const Vector & velocity, Vector & ret) const;

	/*
	 * t: time
	 * pos: position in world frame
	 * velocity: velocity in world frame
	 * ret: velocity in local frame
	 */
	void velocityToLocalFrame(scalar t, const Vector & pos, const Vector & velocity, Vector & ret) const;

	/*
	 * t: time
	 * pos: position in local frame
	 * ret: position in world frame
	 */
	void positionToWorldFrame(scalar t, const Vector & pos, Vector & ret) const;

	/*
	 * t: time
	 * pos: position in world frame
	 * ret: position in local frame
	 */
	void positionToLocalFrame(scalar t, const Vector & pos, Vector & ret) const;

private:
	scalar t0_ = 0.;
	Vector offset_{0, 0, 0};
	Quaternion orientation_{};
	Vector angularVelocity_{0, 0, 0};
};



#endif /* COORDINATESYSTEM_H_ */
