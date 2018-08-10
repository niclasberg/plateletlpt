#ifndef INJECTOR_HH_
#define INJECTOR_HH_
#include "Injector.h"

// InjectorCreator
template<class DerivedType>
std::unique_ptr<Injector> InjectorCreator<DerivedType>::createFromJSON(const json & jsonObject) const
{
	auto p = particleFactory().createFromJSON(jsonObject.at("particle"));
	std::unique_ptr<Injector> inj = std::make_unique<DerivedType>(std::move(p));
	inj->fromJSON(jsonObject);
	return std::move(inj);
}

template<class DerivedType>
std::unique_ptr<Injector> InjectorCreator<DerivedType>::createFromStream(std::istream & in) const
{
	// Read particle
	auto p = particleFactory().createFromStream(in);

	std::unique_ptr<Injector> inj = std::make_unique<DerivedType>(std::move(p));
	inj->readBinary(in);
	return std::move(inj);
}

template<class DerivedType> 
inline int registerInjectorTypeToFactory(const std::string & name)
{
	return injectorFactory().registerCreator(new InjectorCreator<DerivedType>(name));
}

#endif /* INJECTOR_HH_ */
