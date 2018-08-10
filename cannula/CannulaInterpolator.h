#include <fstream>
#include <vector>
#include <cmath>
#include <iostream>
#include <memory>
#include <Eigen/Dense>
#include "Interpolator.h"
#include "io.h"
#include "vtkhelpers.h"
#include "typedefs.h"
#include <random>

// VTK includes
#include <vtkAppendFilter.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkEnSightGoldBinaryReader.h>
#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

class CannulaInterpolator : public UnstructuredPointInterpolator
{
public:
	CannulaInterpolator() : hasRead_(false) { }

	virtual void readData(std::string fileName)
	{
		std::string	cacheFileName = fileName.substr(0, fileName.find_last_of('.')) + ".dat";

		std::cout << "  Reading data from " << fileName << std::endl;
		vtkSmartPointer<vtkEnSightGoldBinaryReader> reader = vtkSmartPointer<vtkEnSightGoldBinaryReader>::New();
		reader->SetCaseFileName(fileName.c_str());
		reader->Update();

		// Extract relevant blocks and merge to one dataset
		/*std::cout << "   Merging blocks" << std::endl;
		vtkSmartPointer<vtkAppendFilter> datasetMerger = vtkSmartPointer<vtkAppendFilter>::New();
		datasetMerger->AddInputData(vtkhelpers::getBlockByName(reader->GetOutput(), "Cannula"));
		datasetMerger->AddInputData(vtkhelpers::getBlockByName(reader->GetOutput(), "CannulaInlet"));
		datasetMerger->Update();*/
		this->ds_ = vtkhelpers::getBlockByName(reader->GetOutput(), "Cannula");

		velocity_ = vtkFloatArray::SafeDownCast(ds_->GetPointData()->GetArray("Velocity"));
		shearXX_  = vtkFloatArray::SafeDownCast(ds_->GetPointData()->GetArray("shearRateii"));
		shearXY_  = vtkFloatArray::SafeDownCast(ds_->GetPointData()->GetArray("shearRateij"));
		shearXZ_  = vtkFloatArray::SafeDownCast(ds_->GetPointData()->GetArray("shearRateik"));
		shearYY_  = vtkFloatArray::SafeDownCast(ds_->GetPointData()->GetArray("shearRatejj"));
		shearYZ_  = vtkFloatArray::SafeDownCast(ds_->GetPointData()->GetArray("shearRatejk"));
		shearZZ_  = vtkFloatArray::SafeDownCast(ds_->GetPointData()->GetArray("shearRatekk"));

		// Check for search tree, build if not present
		//if( ! this->hasSearchTree()) {
			std::cout << "   Building search tree" << std::endl;
			this->buildSearchTree(ds_->GetPoints());			
		//}
		hasRead_ = true;
	}

	virtual CannulaInterpolator * clone() 
	{
		return new CannulaInterpolator(*this);
	}

	virtual bool hasData() const
	{
		return hasRead_;
	}

	virtual bool interpolate(const Vector & position, Vector & velocity, Matrix & shear)
	{
		// Get interpolation weights
		scalar searchRadiusGuess = 1e-4;
		scalar maxSearchRadius = 1e-3;
		std::vector<InterpolationWeight> weights;
		if(this->getInterpolationWeights(position, searchRadiusGuess, maxSearchRadius, weights)) {
			velocity.setZero();
			shear.setZero();
	
			scalar weightSum = 0;
			for(auto weight : weights) {
				// Velocity
				double * velTuple = velocity_->GetTuple(weight.pointId);
				for(int i=0; i < 3; ++i)
					velocity[i] += weight.weight * velTuple[i];

				// Shear rate
				shear(0, 0) += weight.weight * shearXX_->GetTuple1(weight.pointId);
				shear(0, 1) += weight.weight * shearXY_->GetTuple1(weight.pointId);
				shear(0, 2) += weight.weight * shearXZ_->GetTuple1(weight.pointId);
				shear(1, 1) += weight.weight * shearYY_->GetTuple1(weight.pointId);
				shear(1, 2) += weight.weight * shearYZ_->GetTuple1(weight.pointId);
				shear(2, 2) += weight.weight * shearZZ_->GetTuple1(weight.pointId);
				weightSum += weight.weight;
			}
			shear(1, 0) = shear(0, 1);
			shear(2, 0) = shear(0, 2);
			shear(2, 1) = shear(1, 2);

			shear /= weightSum;
			velocity /= weightSum;
			
			return true;
		} else {
			std::cerr << "Interpolation failed" << std::endl;
			return false;
		}
	}

private:
	bool hasRead_;
	vtkFloatArray * velocity_;
	vtkFloatArray * shearXX_;
	vtkFloatArray * shearXY_;
	vtkFloatArray * shearXZ_;
	vtkFloatArray * shearYY_;
	vtkFloatArray * shearYZ_;
	vtkFloatArray * shearZZ_;
	vtkSmartPointer<vtkUnstructuredGrid> ds_;
};

