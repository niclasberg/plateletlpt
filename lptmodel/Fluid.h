#ifndef FLUID_H_
#define FLUID_H_
#include <ostream>
#include "typedefs.h"
#include "macros.h"

class Fluid {
public:
	void fromJSON(const json & jsonObject)
	{
		mu_ = jsonObject.at("mu");
		rho_ = jsonObject.at("rho");
	}

	void readBinary(std::ostream &) { }

	GETSET(scalar, mu)
	GETSET(scalar, rho)
private:
	scalar mu_ = 1e-5;
	scalar rho_ = 1e3;
};

#endif /* FLUID_H_ */
