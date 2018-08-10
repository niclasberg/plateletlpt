#ifndef PARTICLE_H_
#define PARTICLE_H_
#include <Eigen/Dense>
#include "io.h"
#include "macros.h"
#include "Fluid.h"
#include "typedefs.h"
#include <memory>
#include "DynamicFactory.h"
#include "ParticleForces.h"

/* Particle base class */
class Particle {
public:
	virtual ~Particle() { }
	virtual Particle * clone() const = 0;

	// Getters and setters
	GETSET(Vector, position)
	GETSET(Vector, velocity)
	GETSET(Matrix, shear)
	GETSET(scalar, pas)
	GETSET(scalar, dose)
	GETSET(scalar, age)
	GETSET(bool, isAlive)
	GETSET(int, id)
	GETSET(int, collisionCount)
	GETSET(scalar, injectionTime)

	virtual void updateMomentum(scalar dt, const Vector & fluidVelocity, const Matrix & shear, const Fluid & fluid) = 0;
	virtual int typeId() const = 0;
	virtual void fromJSON(const json & jsonObject) { }
	virtual void writeBinary(std::ostream & os) const;
	virtual void readBinary(std::istream & is);

private:
	Vector position_{0., 0., 0.};
	Vector velocity_{0., 0., 0.};
	int id_{-1};
	Matrix shear_;
	scalar pas_{0.};
	scalar dose_{0.};
	scalar age_{0.};
	bool isAlive_{true};
	int collisionCount_{0};
	scalar injectionTime_{-1};
};

/* Particle creation */
using ParticleFactory = DynamicFactory<Particle>;

ParticleFactory & particleFactory();

template<class DerivedType>
inline int registerParticleTypeToFactory(const std::string & typeName)
{
	return particleFactory().registerCreator(new NamedSimpleObjectCreator<Particle, DerivedType>(typeName));
}

/* Tracer particle */
class TracerParticle : public Particle {
public:
	TracerParticle * clone() const override;
	void updateMomentum(scalar dt, const Vector & fluidVelocity, const Matrix & shear, const Fluid & fluid) override;
	int typeId() const override { return typeId_; }

private:
	static int typeId_;
};

/* Material particle */
class MaterialParticle : public Particle {
public:
	MaterialParticle() = default;
	MaterialParticle(const MaterialParticle &);
	MaterialParticle & operator=(const MaterialParticle &);

	GETSET(scalar, density)
	GETSET(scalar, radius)

	MaterialParticle * clone() const override;
	void updateMomentum(scalar dt, const Vector & fluidVelocity, const Matrix & shear, const Fluid & fluid) override;
	int typeId() const override;
	void fromJSON(const json & jsonObject) override;
	void writeBinary(std::ostream & out) const override;
	void readBinary(std::istream & in) override;

private:
	static int typeId_;

	std::vector<std::unique_ptr<ParticleForce>> particleForces_;
	scalar density_{1};
	scalar radius_{1};
};

/* Particle moving with constant velocity (for testing) */
class NoMomentumUpdateParticle : public Particle {
public:
	NoMomentumUpdateParticle * clone() const { return new NoMomentumUpdateParticle(*this); }
	void updateMomentum(scalar dt, const Vector & fluidVelocity, const Matrix & shear, const Fluid & fluid) { }
	virtual int typeId() const { return typeId_; }

private:
	static int typeId_;
};


#endif /* PARTICLE_H_ */
