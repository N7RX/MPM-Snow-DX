#include "pch.h"
#include "Simulator.h"

Simulator::Simulator(Scene* scene) {

	// Convert entities to snow particles
	point_cloud = PointCloud::createEntity(scene->snow_entities);
	if (point_cloud == NULL)
	{
		return;
	}

	// Grid Initialization
	grid = new Grid(
		Eigen::Vector3d(0, 0, 0), 
		Eigen::Vector3d(WIN_METERS_X, WIN_METERS_Y, WIN_METERS_Z), 
		Eigen::Vector3d(GRID_RES_X, GRID_RES_Y, GRID_RES_Z), 
		point_cloud);

	grid->initializeMass();
	grid->calculateVolumes();
}

// Copy constructor
Simulator::Simulator(const Simulator& orig) {}
Simulator::~Simulator() {}


void Simulator::update()
{
	// Rasterize particle mass
	grid->initializeMass();

	// APIC: initialize inertia tensor
	grid->initializeInertiaTensor();

	// Rasterize particle velocity
	grid->initializeVelocities();

	// Compute grid velocities
	grid->explicitVelocities(GRAVITY);

	// APIC: update affine state
	grid->updateAffineState();

	// Map back to particles
	grid->updateVelocities();

	// Update particle data
	point_cloud->update();
}