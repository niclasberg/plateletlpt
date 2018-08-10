#ifndef PARAMETERS_H_
#define PARAMETERS_H_
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <stdexcept>
#include <cstdlib>
#include "io.h"

class ParameterNotFoundException : public std::runtime_error
{
public:
	ParameterNotFoundException(const std::string & cause)
	: std::runtime_error(cause)
	{

	}
};

class Parameters {
public:
	Parameters(std::string fileName)
	{
		std::ifstream s(fileName.c_str());
		if( ! s) {
			std::cerr << "Could not open parameter file " << fileName << std::endl;
		}

		// Read parameters
		std::string line, key, value;
		while(std::getline(s, line)) {
			int spacePos = line.find_first_of(' ');
			key = line.substr(0, spacePos);
			value = line.substr(spacePos+1, std::string::npos);
			params_[key] = value;
		}
	}

	template<class T>
	T read(std::string key)
	{
		T ret;
		read(key, ret);
		return ret;
	}

	template<class T>
	void read(std::string key, T & ret)
	{
		if( ! hasKey(key))
			throw ParameterNotFoundException(stringify("The parameter file had no value with key ", key));
		readImpl(key, ret);
	}

	template<class T>
	T readOrDefault(std::string key, T value)
	{
		T ret;
		readOrDefault(key, value, ret);
		return ret;
	}

	template<class T>
	void readOrDefault(std::string key, T value, T & ret)
	{
		if( ! hasKey(key))
			ret = value;
		else
			readImpl(key, ret);
	}

	bool hasKey(std::string key)
	{
		return params_.find(key) != params_.end();
	}

private:
	void readImpl(std::string key, std::string & ret)
	{
		ret = params_.at(key);
	}

	void readImpl(std::string key, int & ret)
	{
		char * pend;
		ret = std::strtol(params_.at(key).c_str(), &pend, 10);
	}

	void readImpl(std::string key, float & ret)
	{
		char * pend;
		ret = std::strtof(params_.at(key).c_str(), &pend);
	}

	void readImpl(std::string key, double & ret)
	{
		char * pend;
		ret = std::strtod(params_.at(key).c_str(), &pend);
	}

	void readImpl(std::string key, bool & ret)
	{
		if(params_.at(key).compare("1") == 0 || params_.at(key).compare("true") == 0)
			ret = true;
		else if(params_.at(key).compare("0") == 0 || params_.at(key).compare("false") == 0)
			ret = false;
		else
			throw std::runtime_error(stringify("Could not cast the parameter ", key, " to a boolean"));
	}

	std::map<std::string, std::string> params_;
};



#endif /* PARAMETERS_H_ */
