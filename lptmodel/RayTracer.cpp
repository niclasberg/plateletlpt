#include "RayTracer.h"
#include <fstream>
#include "Triangle.h"
#include <iostream>
#include "io.h"
#include <stdexcept>

RayTracer::RayTracer() : bvh_(nullptr), objects_()
{

}

RayTracer::~RayTracer()
{
	clear();
}

void RayTracer::clear()
{
	for(std::vector<Object *>::iterator it = objects_.begin(); it != objects_.end(); ++it)
		delete *it;
	objects_.clear();
	if(bvh_)
		delete bvh_;
}

void RayTracer::fromJSON(const json & j)
{
	std::string stlFile = j.at("stlFile").get<std::string>();

	shouldWrite() = false;
	if(j.count("shouldWrite"))
		shouldWrite() = j.at("shouldWrite").get<bool>();

	CoordinateSystem transform;
	if(j.count("transform"))
		transform.fromJSON(j.at("transform"));
	coordinateSystem() = transform;

	readSTL(stlFile.c_str());
}

void RayTracer::readSTL(const char * fname)
{
	// Clear
	clear();

	// Open input file
	std::ifstream myFile(fname, std::ios::in | std::ios::binary);

	if(!myFile)
		throw std::runtime_error(stringify("Could not open the stl file ", fname));

	char header_info[80] = "";
	unsigned long nTriLong = 0;

	//read 80 byte header
	if (myFile) {
		myFile.read (header_info, 80);
		std::cout << "Reading stl file " << fname << std::endl;
		std::cout << "  Header: " << header_info << std::endl;
	} else {
		std::cerr << "Error reading STL header" << std::endl;
	}

	//read 4-byte ulong
	if (myFile) {
		myFile.read(reinterpret_cast<char * >(&nTriLong), sizeof(unsigned int));
		std::cout << "  Number of triangles: " << nTriLong << std::endl;
	} else {
		std::cerr << "Error reading number of triangles in the STL file" << std::endl;
	}

	//now read in all the triangles
	IntersectionInfo info;
	for(unsigned int i = 0; i < nTriLong; i++){
		char facet[50];
		if (myFile) {
			//read one 50-byte triangle
			myFile.read(facet, 50);

			//populate each point of the triangle
			char * ptr = facet;
			float * data[4];
			for(unsigned int j = 0; j < 4; ++j) {
				data[j] = reinterpret_cast<float*>(ptr);
				ptr += 3*sizeof(float);
			}

			Vector3 normal(data[0][0], data[0][1], data[0][2]);
			Vector3 p1(data[1][0], data[1][1], data[1][2]);
			Vector3 p2(data[2][0], data[2][1], data[2][2]);
			Vector3 p3(data[3][0], data[3][1], data[3][2]);

			//add a new triangle to the array
			Triangle * tri = new Triangle(p1,p2,p3);

			// Reorder indices if the resulting normal points in the wrong direction
			if((tri->getNormal(info) * normal) < 0) {
				std::swap(tri->v0, tri->v1);
			}

			/*std::cout << tri->getNormal(info)[0] << ", " << tri->getNormal(info)[1] << ", " << tri->getNormal(info)[2]
			           << " : " << normal[0] << ", " << normal[1] << ", " << normal[2] << std::endl;
			std::cout << objects_.size() << std::endl;
			std::cout << tri->v0.x << ", " << tri->v0.y << ", " << tri->v0.z << std::endl;
			std::cout << tri->v1.x << ", " << tri->v1.y << ", " << tri->v1.z << std::endl;
			std::cout << tri->v2.x << ", " << tri->v2.y << ", " << tri->v2.z << std::endl;*/

			objects_.push_back(tri);
		}
	}

	bvh_ = new BVH(&objects_);
}

void RayTracer::writeSTL(const char * fname, scalar t) const
{
	std::ofstream myFile(fname, std::ios::binary);

	// Write header
	char header[80];
	std::fill(header, header + 80, ' ');
	myFile.write(header, 80);

	// Write number of elements
	unsigned int numTris = objects_.size();
	write_to_stream(myFile, &numTris, 1);

	// Flags
	char tmp[2];
	std::fill(tmp, tmp+2, ' ');

	// Write triangles
	IntersectionInfo info;
	Vector normal, v0, v1, v2;
	for(unsigned int i = 0; i < numTris; ++i) {
		// Write normal
		const Triangle * tri = reinterpret_cast<Triangle *>(objects_[i]);
		Vector3 n = objects_[i]->getNormal(info);
		coordinateSystem_.normalVectorToWorldFrame(t, Vector(n.x, n.y, n.z), normal);
		coordinateSystem_.positionToWorldFrame(t, Vector(tri->v0.x, tri->v0.y, tri->v0.z), v0);
		coordinateSystem_.positionToWorldFrame(t, Vector(tri->v1.x, tri->v1.y, tri->v1.z), v1);
		coordinateSystem_.positionToWorldFrame(t, Vector(tri->v2.x, tri->v2.y, tri->v2.z), v2);
		write_to_stream(myFile, &(normal[0]), 3);
		write_to_stream(myFile, &(v0[0]), 3);
		write_to_stream(myFile, &(v1[0]), 3);
		write_to_stream(myFile, &(v2[0]), 3);
		myFile.write(tmp, 2);
	}
	myFile.close();
}

/*
 * p0: ray start point (local frame)
 * p1: ray end point (local frame)
 * info: Info about the intersection (if found)
 */
bool RayTracer::findRayIntersection(const Vector & p0, const Vector & p1, IntersectionInfo & info) const
{
	if( ! bvh_)
		return false;

	// Get ray direction
	Vector dir = p1 - p0;
	float dist = dir.norm();
	if(std::abs(dist) < std::numeric_limits<float>::epsilon())
		return false;
	dir /= dist;

	// Check for intersection
	if(bvh_->getIntersection(Ray(Vector3(p0[0], p0[1], p0[2]), Vector3(dir[0], dir[1], dir[2])), &info, false)) {
		return info.t > 0 && info.t <= dist;
	}
	return false;
}

// Naive implementation
/*bool RayTracer::findRayIntersection(const VectorType & p0, const VectorType & p1, IntersectionInfo & info) const
{
	// Create ray
	VectorType dir = p1 - p0;
	float dist = dir.norm();
	if(std::abs(dist) < std::numeric_limits<float>::epsilon()) {
		std::cerr << "WARNING: could not normalize ray direction" << std::endl;
		return false;
	}
	dir /= dist;
	Ray ray(Vector3(p0[0], p0[1], p0[2]), Vector3(dir[0], dir[1], dir[2]));

	info.t = 9999999.f;
	info.object = 0;
	IntersectionInfo info2;
	bool hit_found = false;

	for(unsigned int i = 0; i < objects_.size(); ++i) {
		if(objects_[i]->getIntersection(ray, &info2)) {
			if(info2.t > 0 && info2.t <= dist && info2.t < info.t) {
				info = info2;
				hit_found = true;
			}
		}
	}

	return hit_found;
}*/

