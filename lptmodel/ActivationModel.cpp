#include "ActivationModel.h"
#include "macros.h"
#include <ostream>
#include "typedefs.h"
#include "DynamicFactory.hh"

// Explicit template instantiation of the factory
template class DynamicFactory<ActivationModel>;

ActivationModelFactory & activationModelFactory()
{
	static ActivationModelFactory activationModelFactory;
	return activationModelFactory;
}

// NobiliActivationModel
const int NobiliActivationModel::typeId = registerActivationModelTypeToFactory<NobiliActivationModel>("Nobili");

void NobiliActivationModel::evaluate(float dt, float tau, float & dose, float & pas)
{
	pas += c_ * a_ * dt * std::pow(dose, a_-1) * std::pow(tau, b_/a_);
	dose += dt * std::pow(tau, b_/a_);
}

void NobiliActivationModel::fromJSON(const json & jsonObject) 
{
	a_ = jsonObject.at("a");
	b_ = jsonObject.at("b");
	c_ = jsonObject.at("c");
}

