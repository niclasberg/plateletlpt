#ifndef INPUTFILELIST_H_
#define INPUTFILELIST_H_
#include "typedefs.h"
#include "macros.h"

class InputFileList {
public:
	enum class OutOfRangeMode { Clamp, Repeat, Stop };

	GETSET(OutOfRangeMode, outOfRangeMode)
	GETSET(scalar, dataDt)

	void readFileList(std::string inputFolder);
	void globFiles(std::string inputFolder, std::string suffix);
	void globFiles(std::string inputFolder, std::string suffix, int maxNumberOfFiles);
	bool getDataFileName(size_t dataTimeStep, std::string & fileName) const;

	bool empty() const { return dataFileNames_.empty(); }
	void fromJSON(const json &);
private:
	OutOfRangeMode outOfRangeMode_{OutOfRangeMode::Repeat};
	scalar dataDt_ = 1.;
	std::vector<std::string> dataFileNames_{};
};

#endif
