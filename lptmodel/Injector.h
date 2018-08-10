#ifndef INJECTOR_H_
#define INJECTOR_H_
#include "macros.h"
#include "Particle.h"
#include <Eigen/Dense>
#include "typedefs.h"
#include "DynamicFactory.h"

class Injector {
public:
	explicit Injector(std::unique_ptr<Particle> templateParticle) : templateParticle_(std::move(templateParticle)) { }

	// Hide copy constructor and assignment operator
	Injector(const Injector &) = delete;
	Injector & operator=(const Injector &) = delete;

	virtual void fromJSON(const json & j);
	virtual void readBinary(std::istream &) { }

	void inject(scalar t0, scalar t1, std::vector<Particle *> & pToInject) const;

	GETSET(scalar, tStart)
	GETSET(scalar, tEnd)
	GETSET(int, particlesToInject)

private:
	virtual void getInjectionPositions(scalar t, int numParticles, std::vector<Vector> & positions) const = 0;

	scalar tStart_ = 0.;
	scalar tEnd_ = -1.;
	int particlesToInject_ = 0;
	std::unique_ptr<Particle> templateParticle_;
};

// Creator for Injectors
template<class DerivedType>
class InjectorCreator : public ObjectCreator<Injector> {
public:
	explicit InjectorCreator(const std::string & typeName) : typeName_(typeName) { }
	std::string getObjectTypeName() const override { return typeName_; }
	std::unique_ptr<Injector> createFromJSON(const json &) const override;
	std::unique_ptr<Injector> createFromStream(std::istream &) const override;
private:
	std::string typeName_;
};

using InjectorFactory = DynamicFactory<Injector>;

InjectorFactory & injectorFactory();

template<class DerivedType> int registerInjectorTypeToFactory(const std::string & name);

class BoxInjector : public Injector {
public:
	static const int typeId;

	BoxInjector(std::unique_ptr<Particle> templateParticle) : Injector(std::move(templateParticle)) { }
	
	GETSET(scalar, x0)
	GETSET(scalar, x1)
	GETSET(scalar, y0)
	GETSET(scalar, y1)
	GETSET(scalar, z0)
	GETSET(scalar, z1)

	void fromJSON(const json & jsonObject) override;

private:
	void getInjectionPositions(scalar t, int numParticles, std::vector<Vector> & positions) const override;

	scalar x0_;
	scalar x1_;
	scalar y0_;
	scalar y1_;
	scalar z0_;
	scalar z1_;
};

class CircularInjector : public Injector {
public:
	static const int typeId;

	CircularInjector(std::unique_ptr<Particle> templateParticle)
	: Injector(std::move(templateParticle))
	{ }

	GETSET(Vector, normal)
	GETSET(Vector, origin)
	GETSET(scalar, radius)

	void fromJSON(const json & jsonObject) override;

private:
	void getInjectionPositions(scalar t, int numParticles, std::vector<Vector> & positions) const override;

	Vector normal_{0, 0, 1};
	Vector origin_{0, 0, 0};
	scalar radius_ = 1;
};

#endif /* INJECTOR_H_ */
