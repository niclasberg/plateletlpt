#include "Model.h"
#include "Interpolator.h"
#include "Particle.h"
#include "ActivationModel.h"
#include "Absorber.h"
#include "io.h"
#include "RayTracer.h"
#include "Injector.h"
#include <algorithm>
#include "DynamicFactory.hh"

Model::~Model()
{
	clearParticles();
}

void Model::clearParticles()
{
	particles_.clear();
}

void Model::run()
{
	// Try to load checkpoint
	std::string checkpointFilename = outputFolder() + std::string("checkpoint.dat");
	readCheckpoint(checkpointFilename);

	// Create output folder
	mkdir(outputFolder().c_str());

	while(! isDone()) {
		std::cout << "Iteration " << iteration() << " / " << Nt() << ", time = " << time() << ", #particles = " << particles_.size() << std::endl;

		if((iteration() % outputInterval()) == 0) {
			writeParticles(stringify(outputFolder(), "platelets", iteration(), ".csv"));

			int boundaryId = 0;
			for(auto && r : rayTracers_) {
				if(r->shouldWrite()) {
					r->writeSTL(stringify(outputFolder(), "boundary", boundaryId, "_", iteration(), ".stl").c_str(), time());
				}
				++boundaryId;
			}
		}

		if((iteration() % checkpointInterval()) == 0)
			writeCheckpoint(checkpointFilename);

		update();

		if(iteration() >= Nt())
			isDone_ = true;
	}

	writeCheckpoint(checkpointFilename);
}

void Model::update()
{
	// Remove dead particles
	particles_.erase(
		std::remove_if(particles_.begin(), particles_.end(), 
			[](auto && p) { return !p->isAlive(); }), 
		particles_.end());

	readDataAndUpdateInterpolators();
	injectParticles();
	updateParticles();
	absorbParticles();
	++iteration_;
}

void Model::readDataAndUpdateInterpolators()
{
	if(interpolator_) {
		if(inputFileList_.empty()) {
			std::cerr << "No data files were found!" << std::endl;
		} else {
			// If we perform multiple substeps per iteration, we need the data
			// both for the current and the next timestep
			if(this->substeps() > 1) {
				// If we're at the first subiteration, fetch new data
				if(this->currentSubIteration() == 0 || ! interpolator_->hasData()) {
					std::string nextDataFileName;
					if( ! inputFileList().getDataFileName(this->currentFileId()+1, nextDataFileName)) {
						// Reached end of file list
						isDone_ = true;
						return;
					}

					// InterpolatorNext_ will be null if no data has been read
					if(interpolatorNext_ && interpolatorNext_->hasData()) {
						// Set interpolator to interpolatorNext and read the data at the next iteration
						std::swap(interpolator_, interpolatorNext_);
					} else {
						// We're at the first timestep, read data for both the current and the next iteration
						std::string currentDataFileName;
						if( ! inputFileList().getDataFileName(this->currentFileId(), currentDataFileName)) {
							// Reached end of file list
							isDone_ = true;
							return;
						}
						interpolatorNext_.reset(interpolator_->clone());
						interpolator_->readData(currentDataFileName);
					}
					
					// Read data at next iteration
					interpolatorNext_->readData(nextDataFileName);
				}
			} else {
				// Only one substep, just use the iteration counter as file index
				std::string currentDataFileName;
				if( ! inputFileList().getDataFileName(iteration_, currentDataFileName)) {
					// Reached end of file list
					isDone_ = true;
					return;
				}
				interpolator_->readData(currentDataFileName);
			}
		}
	}
}

void Model::injectParticles()
{
	std::cout << "  Injecting particles" << std::endl;
	std::vector<Particle *> particlesToInject;
	for(auto && injector : injectors_)
		injector->inject(time(), time()+dt(), particlesToInject);

	int numberOfInjectedParticles = 0;
	for(auto && p : particlesToInject) {
		if(interpolator_)
			interpolator_->interpolate(p->position(), p->velocity(), p->shear());
		p->isAlive() = true;
		p->injectionTime() = this->time();
		addParticle(p);

		++numberOfInjectedParticles;
	}
	if(numberOfInjectedParticles > 0)
		std::cout << "   Injected " << numberOfInjectedParticles << " particles" << std::endl;
}

void Model::addParticle(Particle * particle)
{
	particle->id() = nextParticleId_++;
	particles_.emplace_back(particle);
}

void Model::updateParticles()
{
	std::cout << "  Updating particles" << std::endl;
	for(auto && p : particles_) {
		if( ! p->isAlive())
			continue;

		updateParticleMomentumAndActivation(p.get());
		updateParticlePosition(p.get(), time(), time() + dt());
		p->age() += dt();
	}
}

void Model::updateParticleMomentumAndActivation(Particle * p)
{
	if(interpolator_) {
		Matrix shear;
		Vector fluidVelocity;
		if(!interpolator_->interpolate(p->position(), fluidVelocity, shear)) {
			p->isAlive() = false;
			return;
		}

		if(substeps() > 1) {
			// Linear interpolation between the value from the two interpolators
			Matrix shear2;
			Vector fluidVelocity2;				
			if(!interpolatorNext_->interpolate(p->position(), fluidVelocity2, shear2)) {
				p->isAlive() = false;
				return;
			}

			fluidVelocity = (1 - currentTimeStepFraction()) * fluidVelocity + currentTimeStepFraction() * fluidVelocity2;
			shear = (1 - currentTimeStepFraction()) * shear + currentTimeStepFraction() * shear2;
		}

		p->updateMomentum(dt(), fluidVelocity, shear, fluid());

		// Update pas
		scalar tau = std::sqrt(2) * fluid().mu() * shear.norm();
		activationModel_->evaluate(dt(), tau, p->dose(), p->pas());
		p->shear() = shear;
	}
}

void Model::updateParticlePosition(Particle * p, scalar tCurrent, scalar tNext, int collCount)
{
	scalar dtCurrent = tNext - tCurrent;

	Vector pos_last = p->position();
	p->position() += dtCurrent * p->velocity();
	
	if(collCount > 10)
		return;

	// Check if the line between the current and the next position intersects the boundary
	for(auto rtIt = rayTracers_.begin(); rtIt != rayTracers_.end(); ++rtIt) {
		RayTracer & rayTracer = *(*rtIt);

		// Transform positions to the raytracer's coordinate system
		Vector pos_start_rf, pos_end_rf;
		rayTracer.coordinateSystem().positionToLocalFrame(tCurrent, pos_last, pos_start_rf);
		rayTracer.coordinateSystem().positionToLocalFrame(tNext, p->position(), pos_end_rf);

		// Check for collision
		IntersectionInfo intersectionInfo;
		if(rayTracer.findRayIntersection(pos_start_rf, pos_end_rf, intersectionInfo)) {
			// Determine fraction of timestep before collision
			scalar ts_fraction = intersectionInfo.t / (pos_end_rf - pos_start_rf).norm();

			// Time of impact
			scalar tImpact = tCurrent + ts_fraction * dtCurrent;

			// Integrate the position up to the collision moment
			p->position() = pos_last + dtCurrent * ts_fraction * p->velocity();

			// Assume that the collision is fully elastic, so that
			// the normal component of the velocity is reversed
			Vector velocityImpactLocalFrame;
			Vector3 normal1 = intersectionInfo.object->getNormal(intersectionInfo);
			Vector normalLocalFrame(normal1.x, normal1.y, normal1.z);

			rayTracer.coordinateSystem().velocityToLocalFrame(tImpact, p->position(), p->velocity(), velocityImpactLocalFrame);
			velocityImpactLocalFrame -= 2. * (velocityImpactLocalFrame.dot(normalLocalFrame)) * normalLocalFrame;

			// Transform back world frame, and set the particle velocity to the post collision velocity
			rayTracer.coordinateSystem().velocityToWorldFrame(tImpact, p->position(), velocityImpactLocalFrame, p->velocity());

			// Update postion (post collision)
			updateParticlePosition(p, tImpact, tNext, collCount+1);

			p->collisionCount() += 1;
			break;
		}
	}
}

void Model::absorbParticles()
{
	std::cout << "  Absorbing particles" << std::endl;
	int numberOfAbsorbedParticles = 0;
	for(auto && p : particles_)
		if(p->isAlive())
			for(auto && absorber: absorbers_)
				if(absorber->isOutside(p->position())) {
					p->isAlive() = false;
					++numberOfAbsorbedParticles;
				}
	if(numberOfAbsorbedParticles > 0)
		std::cout << "   Absorbed " << numberOfAbsorbedParticles << " particles" << std::endl;
}

void Model::fromJSON(const json & jsonObject)
{
	clearParticles();

	// Read fluid
	std::cout << "Reading fluid properties" << std::endl;
	fluid().fromJSON(jsonObject.at("fluid"));
	
	std::cout << "Reading injectors" << std::endl;
	if(jsonObject.count("injectors"))
		injectors_ = injectorFactory().createVectorFromJSON(jsonObject.at("injectors"));
	else
		injectors_.clear();
	std::cout << " " << injectors_.size() << " injectors read" << std::endl;
	
	// Read absorbers
	std::cout << "Reading absorbers" << std::endl;
	if(jsonObject.count("absorbers"))
		absorbers_ = absorberFactory().createVectorFromJSON(jsonObject.at("absorbers"));
	else
		absorbers_.clear();
	std::cout << " " << absorbers_.size() << " absorbers read" << std::endl;

	// Read activation model
	activationModel_ = activationModelFactory().createFromJSON(jsonObject.at("activation"));

	// Read boundaries
	std::cout << "Reading boundaries" << std::endl;
	rayTracers_.clear();
	if(jsonObject.count("boundaries")) {
		auto & boundaries = jsonObject.at("boundaries");
		if(! boundaries.is_array()) {
			throw std::runtime_error("Expected boundaries to be of type array");
		} else {
			for(auto & childObject : boundaries) {
				auto rayTracer = std::make_unique<RayTracer>();
				rayTracer->fromJSON(childObject);
				rayTracers_.push_back(std::move(rayTracer));
			}
		}
	} else {
		rayTracers_.clear();
	}
	std::cout << " " << rayTracers_.size() << " boundaries read" << std::endl;

	// Read input
	std::cout << "Reading input file properties" << std::endl;
	inputFileList().fromJSON(jsonObject.at("input"));

	// Read output
	{
		const json & outputProperties = jsonObject.at("output");

		outputFolder() = outputProperties.at("folder");
		if(outputFolder()[outputFolder().size()-1] != '/')
			outputFolder().push_back('/');

		outputInterval() = jsonGetOrDefault<int>(outputProperties, "csvOutputInterval", 1);
		checkpointInterval() = jsonGetOrDefault<int>(outputProperties, "checkpointOutputInterval", 100);
	}

	// Read timestepping
	{
		const json & timesteppingProperties = jsonObject.at("timeStepping");
		Nt() = timesteppingProperties.at("numberOfTimesteps").get<int>();
		if(timesteppingProperties.count("subIterations"))
			substeps() = timesteppingProperties.at("subIterations").get<int>();
		else
			substeps() = 1;
	}
}

// Write particle data to a comma separated file
void Model::writeParticles(std::string fileName) const
{
	std::ofstream out(fileName.c_str(), std::ios::out);

	if(out.good()) {
		std::cout << "  Writing data to " << fileName << std::endl;
		// Write header
		out << "id,injection_time,age,x,y,z,ux,uy,uz,pas,tauXX,tauXY,tauXZ,tauYY,tauYZ,tauZZ,dose,isAlive,collisionCount" << std::endl;

		// Write particle data
		for(auto &&  p : particles_) {
			out 	<< p->id() << ","
					<< p->injectionTime() << ","
					<< p->age() << ","
					<< p->position()[0] << "," << p->position()[1] << "," << p->position()[2] << ","
					<< p->velocity()[0] << "," << p->velocity()[1] << "," << p->velocity()[2] << ","
					<< p->pas() << ","
					<< 2*fluid().mu() * p->shear()(0, 0) << ","
					<< 2*fluid().mu() * p->shear()(0, 1) << ","
					<< 2*fluid().mu() * p->shear()(0, 2) << ","
					<< 2*fluid().mu() * p->shear()(1, 1) << ","
					<< 2*fluid().mu() * p->shear()(1, 2) << ","
					<< 2*fluid().mu() * p->shear()(2, 2) << ","
					<< p->dose() << ","
					<< p->isAlive() << ","
					<< p->collisionCount()
					<< std::endl;
		}
	} else {
		std::cout << "  Could not open " << fileName << " for writing" << std::endl;
	}

	out.close();
}

// Checkpointing
void Model::writeCheckpoint(std::string fileName) const
{
	std::ofstream out(fileName.c_str(), std::ios::binary);

	if(out.good()) {
		std::cout << "  Writing checkpoint information to " << fileName << std::endl;
		write_to_stream(out, &iteration_, 1);
		//write_to_stream(out, &dt_, 1);

		// Write number of particles
		int num_particles = particles_.size();
		write_to_stream(out, &num_particles, 1);

		// Dump data
		for(auto && p : particles_)
			p->writeBinary(out);

		out.close();
	} else {
		std::cerr << "  Could not open " << fileName << " for checkpoint writing" << std::endl;
	}
}

void Model::readCheckpoint(std::string fileName)
{
	std::ifstream in(fileName.c_str(), std::ios::binary);

	if(in.good()) {
		// Read iteration
		read_from_stream(in, &iteration_, 1);

		// Read number of particles
		int num_particles;
		read_from_stream(in, &num_particles, 1);

		// Read data
		particles_.clear();
		for(int i = 0; i < num_particles; ++i) {
			particles_.emplace_back(particleFactory().createFromStream(in));
		}

		std::cout << "  Successfully read " << numParticles() << " particles" << std::endl;

		in.close();
	} else {
		std::cerr << "  Could not open the checkpoint file " << fileName << std::endl;
	}
}
