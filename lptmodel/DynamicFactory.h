#ifndef DYNAMICFACTORY_H_
#define DYNAMICFACTORY_H_
#include "DynamicFactory.h"
#include <memory>
#include <iostream>
#include "typedefs.h"
#include <string>

template<class BaseType>
struct ObjectCreator
{
	virtual std::string getObjectTypeName() const = 0;
	virtual std::unique_ptr<BaseType> createFromJSON(const json &) const = 0;
	virtual std::unique_ptr<BaseType> createFromStream(std::istream &) const = 0;
};

/*
Requirements:
- DerivedType inherits from BaseType
- DerivedType has a static string typeName
- DerivedType is default constructible
- BaseType must have a readBinary(std::ostream &) method
- BaseType must have a fromJson(const json &) method
*/
template<class BaseType, class DerivedType>
struct SimpleObjectCreator : public ObjectCreator<BaseType>
{
	std::string getObjectTypeName() const override { return std::string(DerivedType::typeName); }
	std::unique_ptr<BaseType> createFromJSON(const json &) const override;
	std::unique_ptr<BaseType> createFromStream(std::istream &) const override;
};

/*Requirements:
- DerivedType inherits from BaseType
- DerivedType is default constructible
- BaseType must have a readBinary(std::ostream &) method
- BaseType must have a fromJson(const json &) method
*/
template<class BaseType, class DerivedType>
class NamedSimpleObjectCreator : public ObjectCreator<BaseType> {
public:
	NamedSimpleObjectCreator(const std::string & name) : name_(name) { }
	std::string getObjectTypeName() const override { return name_; }
	std::unique_ptr<BaseType> createFromJSON(const json &) const override;
	std::unique_ptr<BaseType> createFromStream(std::istream &) const override;

private:
	std::string name_;
};

template<class BaseType>
class DynamicFactory {
public:
	using BaseClass = BaseType;

	int registerCreator(ObjectCreator<BaseType> * factory);
	std::unique_ptr<BaseType> createFromStream(std::istream & is);
	std::unique_ptr<BaseType> createFromJSON(const json & jsonObject);
	std::vector<std::unique_ptr<BaseType>> createVectorFromJSON(const json & jsonObject);
private:
	std::vector<ObjectCreator<BaseType> *> factories{};
};

#endif
