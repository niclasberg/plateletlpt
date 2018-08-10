#include "Injector.h"
#include "Injector.hh"
#include "DynamicFactory.hh"
#include "io.h"

// Explicit template instantiation of the factory
template class DynamicFactory<Injector>;

InjectorFactory & injectorFactory()
{
	static InjectorFactory injectorFactory;
	return injectorFactory;
}

// Injector
void Injector::fromJSON(const json & jsonObject)
{
	tStart() = jsonGetOrDefault<scalar>(jsonObject, "tStart", 0.);
	tEnd() = jsonObject.at("tEnd").get<scalar>();
	particlesToInject() = jsonObject.at("particlesToInject").get<int>();
}

void Injector::inject(scalar t0, scalar t1, std::vector<Particle *> & pToInject) const
{
	if( ! templateParticle_)
		throw std::runtime_error("No template particle has been assigned to the Injector");

	if(t1 >= tStart() && t0 <= tEnd()) {
		// Determine how many particles to inject
		t0 = std::max(t0, tStart());
		t1 = std::min(t1, tEnd());

		double nParticles = (double) particlesToInject() * (t1 - t0) / (tEnd() - tStart());
		int numberOfParticles = std::floor(nParticles);
		double remainder = nParticles - (double) numberOfParticles;

		// The remainder will be in [0, 1), inject an extra particle with this probability
		if(((double) rand() / RAND_MAX) < remainder)
			numberOfParticles += 1;

		std::vector<Vector> positions;
		getInjectionPositions(tStart(), numberOfParticles, positions);

		for(const Vector & pos : positions) {
			Particle * p = templateParticle_->clone();
			p->position() = pos;
			pToInject.push_back(p);
		}
	}
}

// BoxInjector
const int BoxInjector::typeId = registerInjectorTypeToFactory<BoxInjector>("Box");

void BoxInjector::fromJSON(const json & jsonObject)
{
	Injector::fromJSON(jsonObject);

	x0() = jsonGetOrDefault(jsonObject, "x0", 0);
	x1() = jsonGetOrDefault(jsonObject, "x1", 0);
	y0() = jsonGetOrDefault(jsonObject, "y0", 0);
	y1() = jsonGetOrDefault(jsonObject, "y1", 0);
	z0() = jsonGetOrDefault(jsonObject, "z0", 0);
	z1() = jsonGetOrDefault(jsonObject, "z1", 0);
}

void BoxInjector::getInjectionPositions(scalar t, int numParticles, std::vector<Vector> & positions) const
{
	positions.clear();
	for(int i = 0; i < numParticles; ++i) {
		positions.push_back(Vector(
			x0_ + (x1_ - x0_) * ((scalar)std::rand() / RAND_MAX),
			y0_ + (y1_ - y0_) * ((scalar)std::rand() / RAND_MAX),
			z0_ + (z1_ - z0_) * ((scalar)std::rand() / RAND_MAX)));
	}
}

// CircularInjector
const int CircularInjector::typeId = registerInjectorTypeToFactory<CircularInjector>("Circular");

void CircularInjector::fromJSON(const json & jsonObject)
{
	Injector::fromJSON(jsonObject);

	radius() = jsonObject.at("radius").get<scalar>();
	origin() = jsonGetOrDefault(jsonObject, "origin", Vector(0, 0, 0));
	normal() = jsonGetOrDefault(jsonObject, "normal", Vector(0, 0, 0));
}

void CircularInjector::getInjectionPositions(scalar t, int numParticles, std::vector<Vector> & positions) const
{
	// Basis vectors
	Vector zdir = normal() / normal().norm();

	// Create two vectors orthogonal to zdir
	Vector ydir = Vector(zdir[2], zdir[2], -zdir[0] - zdir[1]) / std::sqrt(2*zdir[2]*zdir[2] + (zdir[0] + zdir[1])*(zdir[0] + zdir[1]));
	Vector xdir = ydir.cross(zdir);

	for(int i = 0; i < numParticles; ++i) {
		// To get an unbiased distribution, sample positions from x in [-r, r] and y in [-r, r] and test
		// that the position is within the circle
		bool success = false;
		scalar xp, yp;
		while(! success) {
			xp = radius_ * (-1.f + 2.f * ((scalar)std::rand() / RAND_MAX));
			yp = radius_ * (-1.f + 2.f * ((scalar)std::rand() / RAND_MAX));
			success = (xp*xp + yp*yp) <= (radius_*radius_);
		}

		positions.push_back(origin_ + xp*xdir + yp*ydir);
	}
}
