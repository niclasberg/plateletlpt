#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
#include "Model.h"
#include <Eigen/Dense>
#include "Interpolator.h"
#include "typedefs.h"

#include "EcmoPumpInterpolator.h"

int main(int argc, char * argv[]) 
{
	// Open parameter file
	std::ifstream in(argv[1]);
	if( ! in.good()) {
		std::cerr << "Could not open parameter file: " << argv[1] << std::endl;
		return EXIT_FAILURE;
	}

	// Create model
	Model model;

	// Read and parse parameters
	json j;
	in >> j;
	model.fromJSON(j);

	// Create interpolator
	model.setInterpolator(new EcmoPumpInterpolator());

	// Run simulation
	model.run();

	return EXIT_SUCCESS;
}
