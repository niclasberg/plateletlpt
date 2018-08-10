#ifndef ABSORBER_H_
#define ABSORBER_H_
#include <Eigen/Dense>
#include "macros.h"
#include "typedefs.h"
#include "DynamicFactory.h"
#include "io.h"
#include <functional>

class Absorber {
public:
	virtual bool isOutside(const Vector & pos) const = 0;
	virtual void fromJSON(const json &) { };
	virtual void readBinary(std::istream &) { }
};

using AbsorberFactory = DynamicFactory<Absorber>;

AbsorberFactory & absorberFactory();

template<class DerivedType>
inline int registerAbsorberTypeToFactory(const std::string & typeName)
{
	return absorberFactory().registerCreator(new NamedSimpleObjectCreator<Absorber, DerivedType>(typeName));
}

template<class Predicate>
class PositionComponentPredicateAbsorber : public Absorber {
public:
	GETSET(int, component)
	GETSET(scalar, value)

	bool isOutside(const Vector & pos) const override
	{
		return Predicate()(pos[component_], value_);
	}

	void fromJSON(const json & jsonObject) override
	{
		component() = jsonObject.at("component");
		if(component() > 2 || component() < 0)
			throw std::runtime_error(stringify("Invalid component ", component(), " expected in range [0, 2]").c_str());
		value() = jsonObject.at("value");
	}

private:
	int component_ = 0;
	scalar value_ = 0.;
}; 

struct PositionComponentLargerThanAbsorber : public PositionComponentPredicateAbsorber<std::greater<scalar>> {
	static const int typeId;
};

struct PositionComponentSmallerThanAbsorber : public PositionComponentPredicateAbsorber<std::less<scalar>> {
	static const int typeId;
};



#endif /* ABSORBER_H_ */
