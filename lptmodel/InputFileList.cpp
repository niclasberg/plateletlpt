#include "InputFileList.h"
#include <boost/filesystem.hpp>
#include "io.h"
#include <fstream>
#include <iostream>

bool InputFileList::getDataFileName(size_t dataFileIndex, std::string & fileName) const
{
	switch(this->outOfRangeMode_) {
	case OutOfRangeMode::Stop:
		if(dataFileIndex >= dataFileNames_.size())
			return false;
		break;
	case OutOfRangeMode::Repeat:
		dataFileIndex = dataFileIndex % dataFileNames_.size();
		break;
	case OutOfRangeMode::Clamp:
		dataFileIndex = std::max(dataFileIndex, dataFileNames_.size()-1);
		break;
	}
	fileName = dataFileNames_[dataFileIndex];
	return true;
}

void InputFileList::fromJSON(const json & jsonObject)
{
	std::string folder = jsonObject.at("folder");

	if(jsonObject.count("outOfRangeMode")) {
		std::string oorMode = jsonObject.at("outOfRangeMode");
		if(oorMode.compare("stop") == 0)
			outOfRangeMode() = OutOfRangeMode::Stop;
		else if(oorMode.compare("clamp") == 0)
			outOfRangeMode() = OutOfRangeMode::Clamp;
		else if(oorMode.compare("repeat") == 0)
			outOfRangeMode() = OutOfRangeMode::Repeat;
		else
			throw std::runtime_error(stringify("Unknown outOfRangeMode:", oorMode).c_str());
	} else 
		outOfRangeMode() = OutOfRangeMode::Repeat;
	
	dataDt() = jsonObject.at("timeBetweenSamples").get<scalar>();

	// Read files
	int maxNumberOfFiles = -1;
	if(jsonObject.count("maxNumberOfFiles"))
		maxNumberOfFiles = jsonObject.at("maxNumberOfFiles");

	if(maxNumberOfFiles == -1)
		globFiles(folder, ".case");
	else
		globFiles(folder, ".case", maxNumberOfFiles);
}


void InputFileList::readFileList(std::string inputFolder)
{
	std::string fileListName = inputFolder + std::string("files.dat");
	std::ifstream in(fileListName.c_str());
	dataFileNames_.clear();
	if( ! in) {
		std::cerr << "Could not open the file list: " << fileListName << std::endl;
		exit(-1);
	} else {
		std::string tmp;
		while(std::getline(in, tmp)) {
			if(!tmp.empty()) {
				dataFileNames_.push_back(inputFolder + tmp);
			}
		}
	}
	in.close();
}

namespace detail {
double get_time(const std::string & f)
{
	// format: /folder/data@iteration.case
	// Locate @ character
	double ret;
	std::stringstream ss(f.substr(f.find_first_of('@')+1));
	ss >> ret;
	return ret;
}

bool compare_time(const std::string & f1, const std::string & f2)
{
	return get_time(f1) < get_time(f2);
}

} /* namespace detail */

void InputFileList::globFiles(std::string inputFolder, std::string suffix)
{
	using namespace boost::filesystem;
	dataFileNames_.clear();

	path dir(inputFolder);
	if(exists(dir)) {
		if(is_directory(dir)) {
			// List all .case files
			for(directory_entry & d : directory_iterator(dir)) {
				if(suffix.compare(d.path().extension().string()) == 0)
					dataFileNames_.push_back(d.path().string());
			}

			// Sort files
			std::sort(dataFileNames_.begin(), dataFileNames_.end(), ::detail::compare_time);
		} else {
			std::cerr << dir << " is not a directory" << std::endl;
		}
	} else {
		std::cerr << dir << " does not exist" << std::endl;
	}
}

void InputFileList::globFiles(std::string inputFolder, std::string suffix, int maxNumberOfFiles)
{
	this->globFiles(inputFolder, suffix);
	if(dataFileNames_.size() > maxNumberOfFiles)
		dataFileNames_.resize(maxNumberOfFiles);
}
