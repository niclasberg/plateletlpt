#include "vtkhelpers.h"
#include <vtkDataObjectTreeIterator.h>
#include <vtkInformation.h>
#include <cstring>
#include <stdexcept>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include "io.h"

vtkUnstructuredGrid * vtkhelpers::getBlockByName(vtkMultiBlockDataSet * data, const std::string & blockName)
{
	vtkUnstructuredGrid * ret = 0;
	vtkDataObjectTreeIterator * iter = data->NewTreeIterator();

	for(iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem()) {
		std::string currentBlockName(iter->GetCurrentMetaData()->Get(vtkMultiBlockDataSet::NAME()));
		bool matchFound = false;

		if(currentBlockName.size() >= blockName.size() && std::strncmp(blockName.c_str(), currentBlockName.c_str(), blockName.size()) == 0) {
			// Check that the rest of the block_name is spaces (otherwise strings such as "foo bar" and "foo     " will match)
			matchFound = true;
			for(int k = blockName.size(); k < currentBlockName.size(); ++k)
				if(currentBlockName[k] != ' ') {
					matchFound = false;
					break;
				}

			if(matchFound) {
				ret = vtkUnstructuredGrid::SafeDownCast(iter->GetCurrentDataObject());
				break;
			}
		}
	}
	iter->Delete();

	if( ! ret)
		throw std::runtime_error(stringify("No block was found with the name ", blockName).c_str());

	return ret;
}

namespace detail {
	template<class T, class VTKDataType>
	Eigen::Map<Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > getVTKArrayAsEigen(vtkDataSetAttributes * data, const char * arrayName)
	{
		if( ! data->HasArray(arrayName))
			throw std::runtime_error((std::string("The array ") + std::string(arrayName) + std::string(" was not found in the dataset")).c_str());
	
		VTKDataType * array = VTKDataType::SafeDownCast(data->GetArray(arrayName));
	
		if( ! array) {
			throw std::runtime_error("Incompatible array type");
		}

		return vtkhelpers::wrap_vtk_array(array);
	}
}

Eigen::Map<Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > vtkhelpers::getFloatArray(vtkDataSetAttributes * data, const char * arrayName)
{
	return detail::getVTKArrayAsEigen<float, vtkFloatArray>(data, arrayName);
}

Eigen::Map<Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > vtkhelpers::getDoubleArray(vtkDataSetAttributes * data, const char * arrayName)
{
	return detail::getVTKArrayAsEigen<double, vtkDoubleArray>(data, arrayName);
}


Eigen::Map<Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > vtkhelpers::wrap_vtk_array(vtkFloatArray * arr)
{
	return Eigen::Map<Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> >(
					static_cast<float *>(arr->GetVoidPointer(0)),
					arr->GetNumberOfTuples(),
					arr->GetNumberOfComponents());
}

Eigen::Map<Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > vtkhelpers::wrap_flattened_vtk_array(vtkFloatArray * arr)
{
	return Eigen::Map<Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> >(
					static_cast<float *>(arr->GetVoidPointer(0)),
					(arr->GetNumberOfTuples()) * (arr->GetNumberOfComponents()),
					1);
}

Eigen::Map<Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > vtkhelpers::wrap_vtk_array(vtkDoubleArray * arr)
{
	return Eigen::Map<Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> >(
					static_cast<double *>(arr->GetVoidPointer(0)),
					arr->GetNumberOfTuples(),
					arr->GetNumberOfComponents());
}
