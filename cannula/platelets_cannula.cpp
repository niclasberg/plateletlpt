#include <fstream>
#include <vector>
#include <list>
#include <sstream>
#include <cmath>
#include <iostream>
#include <memory>
#include "Model.h"
#include "RayTracer.h"
#include "Interpolator.h"
#include "Parameters.h"
#include "io.h"
#include "vtkhelpers.h"
#include "typedefs.h"
#include <random>
#include "DynamicFactory.hh"
#include "Injector.hh"

#include "CannulaInterpolator.h"

// Injector and absorber for the "to ECMO" case
struct CannulaToECMOAbsorber : public Absorber {
	static const int typeId;

	bool isOutside(const Vector & pos) const override
	{
		return pos[2] < 0.001 && (pos[0]*pos[0] + pos[1]*pos[1]) < 3e-3*3e-3;
	}
};
const int CannulaToECMOAbsorber::typeId = registerAbsorberTypeToFactory<CannulaToECMOAbsorber>("CannulaToECMO");


struct CannulaToECMOInjector : public Injector {
	static const int typeId;

	CannulaToECMOInjector(std::unique_ptr<Particle> p) : Injector(std::move(p)) 
	{

	}

	void fromJSON(const json & jsonObject)
	{
		Injector::fromJSON(jsonObject);
		if(jsonObject.count("zLeftInlet"))
			zLeftInlet() = jsonObject.at("zLeftInlet");
		if(jsonObject.count("zRightInlet"))
			zRightInlet() = jsonObject.at("zRightInlet");
	}

	GETSET(scalar, zLeftInlet)
	GETSET(scalar, zRightInlet)

private:
	scalar zLeftInlet_ = 0.01; 
	scalar zRightInlet_ = 170e-3;

	void getInjectionPositions(float t, int numParticles, std::vector<Vector> & positions) const override
	{
		// Here we inject particles at both sides of the surrounding vessel
		positions.clear();
		std::uniform_int_distribution<int> boundaryDistribution(0, 1);
		for(int i = 0; i < numParticles; ++i) {
			float x, y, z;

			// Flip a coin to determine which boundary to inject at
			int injectAt = std::lround(((float)std::rand() / RAND_MAX));

			if(injectAt == 0) {	// Left boundary
				z = zLeftInlet_;
				randomPositionWithRadiusBetween(3.6e-3, 8.9e-3, x, y);
			} else { // Right boundary
				z = zRightInlet_;
				randomPositionWithRadiusBetween(0, 8.9e-3, x, y);
			}

			positions.push_back(Vector(x, y, z));
		}
	}

	void randomPositionWithRadiusBetween(float r0, float r1, float & y, float & z) const {
		float rSqr;
		do {
			// randomize values for x and y in [-r1, r1]
			y = -r1 + 2*r1*((float)std::rand() / RAND_MAX);
			z = -r1 + 2*r1*((float)std::rand() / RAND_MAX);
			rSqr = z*z + y*y;

		} while(rSqr < r0*r0 || rSqr > r1*r1);
	}
};
const int CannulaToECMOInjector::typeId = registerInjectorTypeToFactory<CannulaToECMOInjector>("CannulaToECMO");

int main(int argc, char * argv[])
{
	if(argc < 2) {
		std::cerr << "Usage platelets parameter_file" << std::endl;
		return EXIT_FAILURE;
	}

	// Create model
	Model model;

	// Read parameters
	{
		std::ifstream in(argv[1]);
		if( ! in.good()) {
			std::cerr << "Could not open parameter file: " << argv[1] << std::endl;
			return EXIT_FAILURE;
		}
		json j;
		in >> j;

		// Parse parameters
		model.fromJSON(j);
	}

	// Create interpolator
	model.setInterpolator(new CannulaInterpolator());

	model.run();
}

