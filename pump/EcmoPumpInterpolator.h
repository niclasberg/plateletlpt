// VTK includes
#include "vtkhelpers.h"
#include <vtkAppendFilter.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkEnSightGoldBinaryReader.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>

// Interpolator
class EcmoPumpInterpolator : public UnstructuredPointInterpolator
{
public:
	EcmoPumpInterpolator() : hasRead_(false) { }

	void readData(std::string fileName) override
	{
		std::string	cacheFileName = fileName.substr(0, fileName.find_last_of('.')) + ".dat";

		// Try to read the cache file
		std::ifstream in(cacheFileName.c_str(), std::ios::binary);
		if(in.good()) {
			// Cache file was successfully opened
			std::cout << "   Cache file found, reading from " << cacheFileName << std::endl;
			// Read search tree
			this->readSearchTree(in);			

			// Read point data
			read_from_stream(in, velocity_);
			read_from_stream(in, shearRate_);
			in.close();
	 	} else {
			// Read ensight data
			std::cout << "   Reading data" << std::endl;
			vtkSmartPointer<vtkEnSightGoldBinaryReader> reader = vtkSmartPointer<vtkEnSightGoldBinaryReader>::New();
			reader->SetCaseFileName(fileName.c_str());
			reader->Update();

			//reader->GetOutput()->Print(std::cout);

			// Extract relevant blocks and merge to one dataset
			std::cout << "   Merging blocks" << std::endl;
			vtkSmartPointer<vtkAppendFilter> datasetMerger = vtkSmartPointer<vtkAppendFilter>::New();
			datasetMerger->AddInputData(vtkhelpers::getBlockByName(reader->GetOutput(), "core"));
			datasetMerger->AddInputData(vtkhelpers::getBlockByName(reader->GetOutput(), "volute"));
			datasetMerger->Update();
			vtkUnstructuredGrid * ds = datasetMerger->GetOutput();

			// Resize containers
			std::cout << "   Copying data" << std::endl;
			size_t nPts = ds->GetNumberOfPoints();
			velocity_.resize(nPts, 3);
			shearRate_.resize(nPts, 6);

			// Copy data
			velocity_ 		  = vtkhelpers::getFloatArray(ds->GetPointData(), "Velocity");
			shearRate_.col(0) = vtkhelpers::getFloatArray(ds->GetPointData(), "shearRateii");
			shearRate_.col(1) = vtkhelpers::getFloatArray(ds->GetPointData(), "shearRateij");
			shearRate_.col(2) = vtkhelpers::getFloatArray(ds->GetPointData(), "shearRateik");
			shearRate_.col(3) = vtkhelpers::getFloatArray(ds->GetPointData(), "shearRatejj");
			shearRate_.col(4) = vtkhelpers::getFloatArray(ds->GetPointData(), "shearRatejk");
			shearRate_.col(5) = vtkhelpers::getFloatArray(ds->GetPointData(), "shearRatekk");

			// Build search tree
			std::cout << "   Building search tree" << std::endl;
			this->buildSearchTree(ds->GetPoints());

			// Write to cache file
			std::cout << "   Saving to cache file" << std::endl;
			std::ofstream out(cacheFileName.c_str(), std::ios::binary);
			this->writeSearchTree(out);
			write_to_stream(out, velocity_);
			write_to_stream(out, shearRate_);
			out.close();
		}

		hasRead_ = true;
	}

	virtual bool hasData() const { return hasRead_; }

	virtual EcmoPumpInterpolator * clone()
	{
		return new EcmoPumpInterpolator(*this);
	}

	virtual bool interpolate(const Vector & position, Vector & velocity, Matrix & shear)
	{
		// Get interpolation weights
		scalar searchRadiusGuess = 1e-4;
		scalar maxSearchRadius = 2e-3;
		std::vector<InterpolationWeight > weights;
		if(this->getInterpolationWeights(position, searchRadiusGuess, maxSearchRadius, weights)) {
			velocity.setZero();
			shear.setZero();
	
			scalar weightSum = 0;
			for(auto weight : weights) {
				velocity += weight.weight * velocity_.row(weight.pointId);
				shear(0, 0) += weight.weight * shearRate_(weight.pointId, 0);
				shear(0, 1) += weight.weight * shearRate_(weight.pointId, 1);
				shear(0, 2) += weight.weight * shearRate_(weight.pointId, 2);
				shear(1, 1) += weight.weight * shearRate_(weight.pointId, 3);
				shear(1, 2) += weight.weight * shearRate_(weight.pointId, 4);
				shear(2, 2) += weight.weight * shearRate_(weight.pointId, 5);
				weightSum += weight.weight;
			}
			shear(1, 0) = shear(0, 1);
			shear(2, 0) = shear(0, 2);
			shear(2, 1) = shear(1, 2);

			shear /= weightSum;
			velocity /= weightSum;
			
			return true;
		} else {
			// Fall back to nearest neighbor
			int pointIndx;
			if(this->getNearestNeighbor(position, pointIndx)) {
				velocity = velocity_.row(pointIndx);
				shear(0, 0) = shearRate_(pointIndx, 0);
				shear(0, 1) = shearRate_(pointIndx, 1);
				shear(0, 2) = shearRate_(pointIndx, 2);
				shear(1, 1) = shearRate_(pointIndx, 3);
				shear(1, 2) = shearRate_(pointIndx, 4);
				shear(2, 2) = shearRate_(pointIndx, 5);

				shear(1, 0) = shear(0, 1);
				shear(2, 0) = shear(0, 2);
				shear(2, 1) = shear(1, 2);

				return true;
			}
		}

		std::cerr << "Interpolation failed" << std::endl;
		return false;
	}

private:
	bool hasRead_;
	Eigen::Matrix<float, Eigen::Dynamic, 3> velocity_;
	Eigen::Matrix<float, Eigen::Dynamic, 6> shearRate_;
};
