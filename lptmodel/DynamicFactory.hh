#ifndef DYNAMICFACTORY_HH_
#define DYNAMICFACTORY_HH_
#include <vector>
#include <string>
#include <memory>
#include <locale>
#include <iostream>
#include "typedefs.h"
#include "io.h"

template<class BaseType, class DerivedType>
std::unique_ptr<BaseType> SimpleObjectCreator<BaseType, DerivedType>::createFromJSON(const json & jsonObject) const
{
	std::unique_ptr<BaseType> p = std::make_unique<DerivedType>();
	p->fromJSON(jsonObject);
	return std::move(p);
}

template<class BaseType, class DerivedType>
std::unique_ptr<BaseType> SimpleObjectCreator<BaseType, DerivedType>::createFromStream(std::istream & is) const
{
	std::unique_ptr<BaseType> p = std::make_unique<DerivedType>();
	p->readBinary(is);
	return std::move(p);
}

template<class BaseType, class DerivedType>
std::unique_ptr<BaseType> NamedSimpleObjectCreator<BaseType, DerivedType>::createFromJSON(const json & jsonObject) const
{
	std::unique_ptr<BaseType> p = std::make_unique<DerivedType>();
	p->fromJSON(jsonObject);
	return std::move(p);
}

template<class BaseType, class DerivedType>
std::unique_ptr<BaseType> NamedSimpleObjectCreator<BaseType, DerivedType>::createFromStream(std::istream & is) const
{
	std::unique_ptr<BaseType> p = std::make_unique<DerivedType>();
	p->readBinary(is);
	return std::move(p);
}


template<class BaseType>
int DynamicFactory<BaseType>::registerCreator(ObjectCreator<BaseType> * factory)
{
	factories.push_back(factory);
	return factories.size()-1;
}

template<class BaseType>
inline std::unique_ptr<BaseType> DynamicFactory<BaseType>::createFromStream(std::istream & is)
{
	int typeId;
	read_from_stream(is, typeId);

	if(typeId < 0 || typeId >= factories.size())
		throw std::runtime_error("Invalid typeId");

	return std::move(factories[typeId]->createFromStream(is));
}

namespace detail{
inline bool equalIgnoreCase(const std::string & a, const std::string & b)
{
	return std::equal(
		a.begin(), a.end(),
		b.begin(), b.end(),
		[](char ac, char bc) -> bool{ 
			return std::tolower(ac) == std::tolower(bc); 
		}
	);
}
}

template<class BaseType>
inline std::unique_ptr<BaseType> DynamicFactory<BaseType>::createFromJSON(const json & jsonObject)
{
	// Get type name
	std::string typeName;
	{
		auto it = jsonObject.find("type");
		if(it == jsonObject.end())
			throw std::runtime_error("No type specified");
		if( ! it->is_string())
			throw std::runtime_error("Expected 'type' to be a string!");

		typeName = *it;
	}
	
	auto it = std::find_if(factories.begin(), factories.end(), [&](auto && factory) { 
		return detail::equalIgnoreCase(factory->getObjectTypeName(), typeName); 
	});

	if(it == factories.end()) {	
		std::stringstream ss;
		ss << "Unknown type name " << typeName << std::endl;
		ss << "Expected one of:";
		for(auto && factory : factories) {
			ss << std::endl << factory->getObjectTypeName();
		}
		ss << "Number of factories" << factories.size();
		
		throw std::runtime_error(ss.str().c_str());
	}

	return std::move((*it)->createFromJSON(jsonObject));
}

template<class BaseType>
inline std::vector<std::unique_ptr<BaseType>> DynamicFactory<BaseType>::createVectorFromJSON(const json & jsonObject)
{
	std::vector<std::unique_ptr<BaseType>> ret;
	if(! jsonObject.is_array())
		std::runtime_error("Expected type to be array");
	else
		for(auto & childObject : jsonObject)
			ret.emplace_back(this->createFromJSON(childObject));
	return std::move(ret);
}

/*template<class Factory, class DerivedType>
inline int registerObjectTypeToDynamicFactory()
{
	return Factory::registerCreator(new SimpleObjectCreator<typename Factory::BaseClass, DerivedType>);
}

template<class Factory, class DerivedType> 
inline int registerObjectTypeToDynamicFactory(const std::string & name)
{
	return Factory::registerCreator(new NamedSimpleObjectCreator<typename Factory::BaseClass, DerivedType>(name));
}*/

#endif
