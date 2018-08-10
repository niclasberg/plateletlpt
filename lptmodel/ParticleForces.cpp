#include "ParticleForces.h"
#include <ostream>
#include "typedefs.h"
#include <algorithm>
#include "DynamicFactory.hh"

// Explicit instantiation of the factory
template class DynamicFactory<ParticleForce>;

ParticleForceFactory & particleForceFactory()
{
	static ParticleForceFactory particleForceFactory;
	return particleForceFactory;
}

// ParticleForce
void ParticleForce::writeBinary(std::ostream & os) const
{
	write_to_stream(os, this->typeId());
}

// StokesDrag
Vector StokesDrag::getParticleForce(const ParticleForceData & forceData)
{
	return 6. * M_PI * forceData.fluid.mu() * forceData.particleRadius * (forceData.fluidVelocity - forceData.particleVelocity);
}

int StokesDrag::typeId_ = registerParticleForceTypeToFactory<StokesDrag>("StokesDrag");
