#ifndef ACTIVATIONMODEL_H_
#define ACTIVATIONMODEL_H_
#include "ActivationModel.h"
#include "DynamicFactory.hh"

template<class DerivedType>
int registerActivationModelTypeToFactory(const std::string & typeName)
{
	return activationModelFactory().registerCreator(new NamedSimpleObjectCreator<ActivationModel, DerivedType>(typeName));
}

#endif /* ACTIVATIONMODEL_H_ */
