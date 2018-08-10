#ifndef Triangle_h_
#define Triangle_h_

#include <cmath>
#include "Object.h"
#include <limits>

struct Triangle : public Object {
	Vector3 v0, v1, v2;

	Triangle(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2)
	: v0(v0), v1(v1), v2(v2)
	{ }

	/*bool getIntersection(const Ray& ray, IntersectionInfo* I) const {
		const Vector3 e1 = v1-v0;
		const Vector3 e2 = v2-v0;
		const Vector3 q  = ray.d ^ e2;
		const float det  = e1 * q;

		// Check if the vector is parallel to the plane (the intersection is at infinity)
		if (det > -std::numeric_limits<float>::epsilon() && det < std::numeric_limits<float>::epsilon())
			return false;

		const float invDet = 1.f / det;
		const Vector3 s = ray.o - v0;
		const float u = (s * q) * invDet;

		// Check if the intersection is outside of the triangle
		if (u < 0.f || u > 1.f)
			return false;

		const Vector3 r = s ^ e1;
		const float v = (ray.d * r) * invDet;

		// Check if the intersection is outside of the triangle
		if(v < 0.f || (u+v) > 1.f)
			return false;

		I->object = this;
		I->t = (e2 * r) * invDet;
		I->hit = ray.o + I->t*ray.d;
		if(I->t > 0.f)
			return true;
		else
			return false;
	}*/

	bool getIntersection(const Ray& ray, IntersectionInfo* I) const {
		Eigen::Vector3d v0d(v0.x, v0.y, v0.z), v1d(v1.x, v1.y, v1.z), v2d(v2.x, v2.y, v2.z);
		Eigen::Vector3d dir(ray.d.x, ray.d.y, ray.d.z);

		const Eigen::Vector3d e1 = v1d-v0d;
		const Eigen::Vector3d e2 = v2d-v0d;
		const Eigen::Vector3d q  = dir.cross(e2);
		const double a  = e1.dot(q);

		// Check if the vector is parallel to the plane (the intersection is at infinity)
		if (a > -std::numeric_limits<double>::epsilon() && a < std::numeric_limits<double>::epsilon())
			return false;

		const double f = 1. / a;
		const Eigen::Vector3d s = Eigen::Vector3d(ray.o.x, ray.o.y, ray.o.z) - v0d;
		const double u = f * s.dot(q);

		// Check if the intersection is outside of the triangle
		if (u < 0.0)
			return false;

		const Eigen::Vector3d r = s.cross(e1);
		const double v = f*dir.dot(r);

		// Check if the intersection is outside of the triangle
		if(v < 0. || (u+v) > 1.)
			return false;

		double dist = (f * e2.dot(r));
		if(dist < 0.)
			return false;

		I->object = this;
		I->t = (float) dist;
		I->hit = ray.o + I->t*ray.d;
		return true;
	}

	/*bool getIntersection(const Ray& ray, IntersectionInfo* I) const {
		// Intersection test based on Woop et al. (2013) Watertight Ray/Triangle intersection 
		// Determine maximal ray direction
		int maxDim;
		if(ray.d[0] > ray.d[1] && ray.d[0] > ray.d[2])
			maxDim = 0;
		else if(ray.d[1] > ray.d[0] && ray.d[1] > ray.d[2])
			maxDim = 1;
		else
			maxDim = 2;

		int kz = maxDim;
		int kx = kz+1; if(kx == 3) kx = 0;
		int ky = kx+1; if(ky == 3) ky = 0;

		// Swap kx and ky to preserve the winding direction of the triangle
		if(ray.d[kz] < 0.f) std::swap(kx, ky);

		// Shear constants
		float Sx = ray.d[kx]/ray.d[kz];
		float Sy = ray.d[ky]/ray.d[kz];
		float Sz = 1.0f / ray.d[kz];

		// Translate vertices to ray origin
		const Vector3 a = v0 - ray.o;
		const Vector3 b = v1 - ray.o;
		const Vector3 c = v2 - ray.o;
		
		// Shear and scale vertices
		const float ax = a[kx] - Sx*a[kz];
		const float ay = a[ky] - Sy*a[kz];
		const float bx = b[kx] - Sx*b[kz];
		const float by = b[ky] - Sy*b[kz];
		const float cx = c[kx] - Sx*c[kz];
		const float cy = c[ky] - Sy*c[kz];

		// Barycentric coordinates
		float u = cx*by - cy*bx;
		float v = ax*cy - ay*cx;
		float w = bx*ay - by*ax;

		// Fallback to double precision if needed
		if(u == 0.f || v == 0.f || w == 0.f) {
			u = (float)((double)cx*(double)by - (double)cy*(double)bx);
			v = (float)((double)ax*(double)cy - (double)ay*(double)cx);
			w = (float)((double)bx*(double)ay - (double)by*(double)ax);
		}

		if(u < 0.f || v < 0.f || w < 0.f)
			return false;

		// Check if the ray is coplanar to the triangle
		float det = u+v+w;
		if(det == 0.f)
			return false;

		// Calculate hit distance
		const float az = Sz * a[kz];
		const float bz = Sz * b[kz];
		const float cz = Sz * c[kz];
		const float dist = u*az + v*bz + w*cz;

		if(dist < 0.f)
			return false;

		I->object = this;
		I->t = dist / det;
		I->hit = ray.o + I->t * ray.d;
	}*/

	

	Vector3 getNormal(const IntersectionInfo& I) const {
		Vector3 ret = (v1-v0)^(v2-v0);
		float len = length(ret);
		assert(len > 0 && "TRIANGLE NORMAL HAS LENGTH 0!");
		return ret / len;
	}

	BBox getBBox() const {
		Vector3 minimum(std::min(v0.x, std::min(v1.x, v2.x)),
						std::min(v0.y, std::min(v1.y, v2.y)),
						std::min(v0.z, std::min(v1.z, v2.z)));
		Vector3 maximum(std::max(v0.x, std::max(v1.x, v2.x)),
						std::max(v0.y, std::max(v1.y, v2.y)),
						std::max(v0.z, std::max(v1.z, v2.z)));

		// Enlarge the bounding box slightly
		float diagonal = length(maximum - minimum);
		for(int i = 0; i < 3; ++i) {
			minimum[i] -= 0.01*diagonal;
			maximum[i] += 0.01*diagonal;
		}
		return BBox(minimum, maximum);

	}

	Vector3 getCentroid() const {
		return (v0+v1+v2)/3.f;
	}

};

#endif
