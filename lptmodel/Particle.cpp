#include "Particle.h"
#include "io.h"
#include "macros.h"
#include "Fluid.h"
#include "typedefs.h"
#include "DynamicFactory.hh"

// Explicit instantiation of factory
//template class DynamicFactory<Particle>;

ParticleFactory & particleFactory()
{
	static ParticleFactory particleFactory;
	return particleFactory;
}

// Particle
void Particle::writeBinary(std::ostream & os) const
{
	write_to_stream(os, typeId());
	write_to_stream(os, id_);
	write_to_stream(os, position());
	write_to_stream(os, velocity());
	write_to_stream(os, shear_);
	write_to_stream(os, pas_);
	write_to_stream(os, dose_);
	write_to_stream(os, age_);
	write_to_stream(os, isAlive_);
	write_to_stream(os, collisionCount_);
	write_to_stream(os, injectionTime_);
}

void Particle::readBinary(std::istream & is)
{
	read_from_stream(is, id_);
	read_from_stream(is, position());
	read_from_stream(is, velocity());
	read_from_stream(is, shear_);
	read_from_stream(is, pas_);
	read_from_stream(is, dose_);
	read_from_stream(is, age_);
	read_from_stream(is, isAlive_);
	read_from_stream(is, collisionCount_);
	read_from_stream(is, injectionTime_);
}

// Tracer particle
int TracerParticle::typeId_ = registerParticleTypeToFactory<TracerParticle>("TracerParticle");

TracerParticle * TracerParticle::clone() const
{ 
	return new TracerParticle(*this); 
}

void TracerParticle::updateMomentum(scalar dt, const Vector & fluidVelocity, const Matrix & shear, const Fluid & fluid)
{
	this->velocity() = fluidVelocity;
}

// Material particle
int MaterialParticle::typeId_ = registerParticleTypeToFactory<MaterialParticle>("MaterialParticle");

MaterialParticle::MaterialParticle(const MaterialParticle & rhs)
: density_(rhs.density_), radius_(rhs.radius_)
{
	// Clone particle forces
	for(auto && particleForce : rhs.particleForces_)
		particleForces_.emplace_back(std::unique_ptr<ParticleForce>(particleForce->clone()));
}

MaterialParticle & MaterialParticle::operator=(const MaterialParticle & rhs)
{
	if(this != &rhs) {
		particleForces_.clear();
		for(auto && particleForce : rhs.particleForces_)
			particleForces_.emplace_back(std::unique_ptr<ParticleForce>(particleForce->clone()));
		radius_ = rhs.radius_;
		density_ = rhs.density_;
	}
	return *this;
}

MaterialParticle * MaterialParticle::clone() const
{ 
	return new MaterialParticle(*this); 
}

void MaterialParticle::updateMomentum(scalar dt, const Vector & fluidVelocity, const Matrix & shear, const Fluid & fluid)
{
	// Compute total force
	Vector totalForce(0, 0, 0);
	ParticleForceData forceData{fluidVelocity, velocity(), shear, fluid, radius(), density()};

	for(auto && particleForce : particleForces_)
		totalForce += particleForce->getParticleForce(forceData);
	
	this->velocity() += 3. * dt * totalForce / (4. * density_ * radius_ * radius_ * radius_ * M_PI);
}

int MaterialParticle::typeId() const
{ 
	return typeId_; 
}

void MaterialParticle::fromJSON(const json & jsonObject)
{
	Particle::fromJSON(jsonObject);
	density() = jsonObject.at("density");
	radius() = jsonObject.at("radius");
	particleForces_ = particleForceFactory().createVectorFromJSON(jsonObject.at("forces"));
}

void MaterialParticle::writeBinary(std::ostream & out) const
{
	Particle::writeBinary(out);
	write_to_stream(out, density_);
	write_to_stream(out, radius_);

	// Write particle forces
	write_to_stream<int>(out, particleForces_.size());
	for(auto && particleForce : particleForces_)
		particleForce->writeBinary(out);
}

void MaterialParticle::readBinary(std::istream & in)
{
	Particle::readBinary(in);
	read_from_stream(in, density_);
	read_from_stream(in, radius_);

	int numberOfParticleForces;
	particleForces_.clear();
	read_from_stream(in, numberOfParticleForces);
	for(int i = 0; i < numberOfParticleForces; ++i)
		particleForces_.emplace_back(particleForceFactory().createFromStream(in));
}

// No momentum update particle
int NoMomentumUpdateParticle::typeId_ = registerParticleTypeToFactory<NoMomentumUpdateParticle>("NoMomentumUpdateParticle");
	
