#ifndef VTKHELPERS_H_
#define VTKHELPERS_H_

#include <vtkUnstructuredGrid.h>
#include <vtkMultiBlockDataSet.h>

#include <string>
#include <Eigen/Core>

class vtkDoubleArray;
class vtkFloatArray;
class vtkDataSetAttributes;

namespace vtkhelpers {

vtkUnstructuredGrid * getBlockByName(vtkMultiBlockDataSet * data, const std::string & blockName);

// Wrap vtk arrays, and represent the data as an Eigen matrix
Eigen::Map<Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > wrap_vtk_array(vtkFloatArray * arr);
Eigen::Map<Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > wrap_flattened_vtk_array(vtkFloatArray * arr);
Eigen::Map<Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > wrap_vtk_array(vtkDoubleArray * arr);

// Get arrays from PointData or CellData containers as Eigen matrices
Eigen::Map<Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > getFloatArray(vtkDataSetAttributes * data, const char * arrayName);
Eigen::Map<Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > getDoubleArray(vtkDataSetAttributes * data, const char * arrayName);

} /* namespace vtkhelpers */

#endif
