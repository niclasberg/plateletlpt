#ifndef INTERPOLATOR_H_
#define INTERPOLATOR_H_
#include <Eigen/Dense>
#include <string>
#include <vector>
#include "macros.h"
#include <stdexcept>
#include "typedefs.h"
#include <iostream>
#include "kd-tree.h"

#include <vtkPoints.h>

class Interpolator {
public:
	virtual ~Interpolator()
	{

	}
	
	virtual Interpolator * clone() = 0;

	// Velocity and shear interpolation
	virtual bool interpolate(const Vector & position, Vector & velocity, Matrix & shear) = 0;

	// Read next file
	virtual void readData(std::string) = 0;

	virtual bool hasData() const = 0;
};

struct InterpolationWeight {
	int pointId;
	scalar weight;
};

class UnstructuredPointInterpolator : public Interpolator {
public:
	using SearchTree = kd_tree<scalar, 3>;
	using KDVectorType = feature_vector<scalar, 3>;

	UnstructuredPointInterpolator() : searchTree_(nullptr) { }
	UnstructuredPointInterpolator(const UnstructuredPointInterpolator & rhs) : searchTree_(nullptr)
	{
		// Don't copy the search tree	
	}

	void buildSearchTree(vtkPoints * points) 
	{
		KDVectorType pos;
		double vtkPos[3];
		std::vector<KDVectorType> positions;
		positions.reserve(points->GetNumberOfPoints());
		for(int j = 0; j < points->GetNumberOfPoints(); ++j) {
			points->GetPoint(j, vtkPos);
			pos[0] = vtkPos[0]; pos[1] = vtkPos[1]; pos[2] = vtkPos[2]; 
			positions.push_back(pos);
		}
		buildSearchTree(positions);
	}

	void buildSearchTree(const std::vector<KDVectorType> & points)
	{
		searchTree_.reset(new SearchTree);
		searchTree_->build(&(points[0]), points.size());
	}
	
	void writeSearchTree(std::ostream & out) const
	{
		out << searchTree();
	}
	void readSearchTree(std::istream & in)
	{
		searchTree_.reset(new SearchTree);
		in >> searchTree();
	}

	bool getNearestNeighbor(const Vector & position, int & neighborIndex) const
	{
		KDVectorType pos;
		pos[0] = position[0]; pos[1] = position[1]; pos[2] = position[2];

		std::vector<typename SearchTree::kd_neighbour> neighbors;
		searchTree().knn(pos, 1, neighbors);

		if(neighbors.size() <= 0) {
			std::cerr << "No neighbors found!" << std::endl;
			return false;
		}

		neighborIndex = neighbors[0].index;

		return true;
	}

	bool getInterpolationWeights(const Vector & position, scalar minSearchRadius, scalar maxSearchRadius, std::vector<InterpolationWeight> & weights) const
	{
		KDVectorType pos;
		pos[0] = position[0]; pos[1] = position[1]; pos[2] = position[2];

		// First test if an exact match can be found
		std::vector<typename SearchTree::kd_neighbour> neighbors;
		searchTree().knn(pos, 1, neighbors);
		if(neighbors.size() > 0 && sqrtf(neighbors[0].squared_distance) < std::numeric_limits<float>::epsilon()) {
			// Exact match found
			InterpolationWeight intWeight;
			intWeight.weight = 1.;
			intWeight.pointId = neighbors[0].index;
			weights.push_back(intWeight);
			return true;
		}

		// No exact match, use Shepard interpolation (inverse distance weighted interpolation)
		// Test the minimal search radius
		scalar searchRadius;
		neighbors.clear(); 	
		searchTree().all_in_range(pos, minSearchRadius, neighbors, false);
		// If we get 3 or more points, use them for interpolation
		if(neighbors.size() >= 3) {
			searchRadius = minSearchRadius;
		} else {
			// Test the max radius
			neighbors.clear(); 	
			searchTree().all_in_range(pos, maxSearchRadius, neighbors, false);
			if(neighbors.size() == 0) {
				return false;
			}

			// We prefer to have between 3 and 5 search points, if the max search radius yields less that 3, just stick with it
			// Otherwise, do interval halving to find a searchRadius that yields between 3 and 5 points
			if(neighbors.size() <= 3) {
				searchRadius = maxSearchRadius;
			} else {
				scalar minRadius = minSearchRadius;
				scalar maxRadius = maxSearchRadius;
				while(true) {
					searchRadius = 0.5*(minRadius + maxRadius);
					neighbors.clear(); 	
					searchTree().all_in_range(pos, searchRadius, neighbors, false);

					if(neighbors.size() > 5)
						maxRadius = searchRadius;
					else if(neighbors.size() < 3)
						minRadius = searchRadius;
					else
						break; // done
				}
			}
		}

		// Form weights
		for(unsigned int k = 0; k < neighbors.size(); ++k) {
			scalar dist = std::sqrt(neighbors[k].squared_distance);
			scalar weight = std::max(0.f, searchRadius - dist) / (searchRadius*dist);
					
			InterpolationWeight intWeight;
			intWeight.weight = weight * weight;
			intWeight.pointId = neighbors[k].index;
			weights.push_back(intWeight);
		}

		return true;
	}

	bool hasSearchTree() const 
	{
		return (bool) searchTree_;
	}

	SearchTree & searchTree() 
	{ 
		if(! searchTree_)
			std::runtime_error("The search tree has not been initalized");
		return *searchTree_; 
	}
	
	const SearchTree & searchTree() const
	{ 
		if(! searchTree_)
			std::runtime_error("The search tree has not been initalized");
		return *searchTree_; 
	}

private:
	std::unique_ptr<SearchTree> searchTree_;
};


#endif /* INTERPOLATOR_H_ */
