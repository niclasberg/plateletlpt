#ifndef ACTIVATIONMODEL_H_
#define ACTIVATIONMODEL_H_
#include "macros.h"
#include <ostream>
#include "typedefs.h"
#include "DynamicFactory.h"

class ActivationModel {
public:
	virtual void evaluate(float dt, float tau, float & dose, float & pas) = 0;
	virtual void fromJSON(const json &) = 0;
	virtual void readBinary(std::istream &) { }
};

using ActivationModelFactory = DynamicFactory<ActivationModel>;
ActivationModelFactory & activationModelFactory();

template<class DerivedType>
inline int registerActivationModelTypeToFactory(const std::string & typeName)
{
	return activationModelFactory().registerCreator(new NamedSimpleObjectCreator<ActivationModel, DerivedType>(typeName));
}

class NobiliActivationModel : public ActivationModel {
public:
	static constexpr const char * typeName{"Nobili"};
	static const int typeId;

	void evaluate(float dt, float tau, float & dose, float & pas) override;
	void fromJSON(const json & jsonObject) override;

	GETSET(float, a)
	GETSET(float, b)
	GETSET(float, c)

private:
	float a_, b_, c_;
};

#endif /* ACTIVATIONMODEL_H_ */
