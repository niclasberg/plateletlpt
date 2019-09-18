#ifndef MODEL_H_
#define MODEL_H_
#include <memory>
#include "macros.h"
#include <Eigen/Dense>
#include "Fluid.h"
#include "typedefs.h"
#include "Interpolator.h"
#include "RayTracer.h"
#include "Interpolator.h"
#include "Injector.h"
#include "Absorber.h"
#include "InputFileList.h"
#include "ActivationModel.h"

class Model {
public:
	Model() = default;
	~Model();

	// Disallow copy construction and assignment
	Model(const Model &) = delete;
	Model & operator=(const Model &) = delete;

	void addRayTracer(RayTracer * rayTracer) { rayTracers_.push_back(std::unique_ptr<RayTracer>(rayTracer)); }
	void clearRayTracers() { rayTracers_.clear(); }
	void addInjector(Injector * injector) { injectors_.push_back(std::unique_ptr<Injector>(injector)); }
	void clearInjectors() { injectors_.clear(); }
	void addParticle(Particle * particle);
	void clearParticles();
	void addAbsorber(Absorber * absorber) { absorbers_.push_back(std::unique_ptr<Absorber>(absorber)); }
	void clearAbsorbers() { absorbers_.clear(); }
	void setInterpolator(Interpolator * interpolator) { interpolator_.reset(interpolator); interpolatorNext_.reset(nullptr); }
	void setActivationModel(ActivationModel * activationModel) { activationModel_.reset(activationModel); }
	bool isDone() const { return isDone_; }

	GETSET(int, Nt)
	GETSET(int, outputInterval)
	GETSET(int, checkpointInterval)
	GETSET(int, iteration)
	GETSET(Fluid, fluid)
	GETSET(int, substeps)
	GETSET(InputFileList, inputFileList)
	GETSET(std::string, outputFolder)

	int numParticles() const { return particles_.size(); }
	scalar time() const { return iteration() * dt(); }
	scalar dataDt() const { return inputFileList().dataDt(); }
	scalar dt() const { return dataDt() / (scalar) substeps(); }

	void run();
	void update();

	// IO
	void writeParticles(std::string) const;
	void readParticles(std::string);
	void writeCheckpoint(std::string) const;
	void readCheckpoint(std::string);
	void fromJSON(const json &);

private:
	// Indexing helpers
	scalar currentTimeStepFraction() const { return (scalar) currentSubIteration() / (scalar) substeps(); }
	int currentSubIteration() const { return iteration() % substeps(); }
	int currentFileId() const { return iteration() / substeps(); /* integer division rounds towards zero*/ }

	void updateParticles();
	void injectParticles();
	void absorbParticles();
	void readDataAndUpdateInterpolators();
	void updateParticleMomentumAndActivation(Particle * p);
	void updateParticlePosition(Particle * p, scalar, scalar, int collCount = 0);

	bool isDone_ = false;
	int substeps_ = 1;
	int iteration_ = 0;
	int nextParticleId_ = 0;
	int Nt_ = 0;
	int outputInterval_ = 1;
	int checkpointInterval_ = 1;
	std::string outputFolder_{"."};
	InputFileList inputFileList_{};
	std::unique_ptr<Interpolator> interpolator_{nullptr};
	std::unique_ptr<Interpolator> interpolatorNext_{nullptr};
	std::unique_ptr<ActivationModel> activationModel_{nullptr};
	std::vector<std::unique_ptr<RayTracer>> rayTracers_{};
	std::vector<std::unique_ptr<Injector>> injectors_{};
	std::vector<std::unique_ptr<Absorber>> absorbers_{};
	std::vector<std::unique_ptr<Particle>> particles_{};
	Fluid fluid_{};
};

#endif /* MODEL_H_ */
