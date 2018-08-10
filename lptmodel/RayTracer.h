#ifndef RAYTRACER_H_
#define RAYTRACER_H_
#include "BVH.h"
#include "Object.h"
#include "Quaternion.h"
#include "CoordinateSystem.h"
#include "typedefs.h"
#include "macros.h"

class RayTracer {
public:
	RayTracer();
	virtual ~RayTracer();
	RayTracer(const RayTracer &) = delete;
	RayTracer & operator=(const RayTracer &) = delete;

	GETSET(CoordinateSystem, coordinateSystem)
	GETSET(bool, shouldWrite)

	void readSTL(const char *);
	void writeSTL(const char *, scalar) const;
	void fromJSON(const json &);

	void clear();
	bool findRayIntersection(const Vector &, const Vector &, IntersectionInfo &) const;

private:
	bool shouldWrite_ = false;
	CoordinateSystem coordinateSystem_;
	BVH * bvh_ = nullptr;
	std::vector<Object *> objects_{};
};

#endif /* RAYTRACER_H_ */
