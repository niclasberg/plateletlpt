#include "Absorber.h"
#include "DynamicFactory.hh"

// Explicit instantiation of the factory
template class DynamicFactory<Absorber>;

AbsorberFactory & absorberFactory()
{
	static AbsorberFactory absorberFactory;
	return absorberFactory;
}

// PositionComponentLargerThanAbsorber
const int PositionComponentLargerThanAbsorber::typeId = registerAbsorberTypeToFactory<PositionComponentLargerThanAbsorber>("PositionComponentLargerThan");

// PositionComponentSmallerThanAbsorber
const int PositionComponentSmallerThanAbsorber::typeId = registerAbsorberTypeToFactory<PositionComponentSmallerThanAbsorber>("PositionComponentSmallerThan");
