#pragma once
#ifndef PARTICLE_H
#define PARTICLE_H

#include <cmath>
#include "CustomMath.h"
#include "SimulationParameters.h"

class Particle
{
public:
	double volume, mass, density;
	Eigen::Vector3d position, velocity;
	Eigen::Matrix3d velocity_gradient;

	// APIC: locally affine matrix
	// The velocity derivatives matrix C equals B*D^-1
	// namely, affine_state * intertia_tensor_reverse
	Eigen::Matrix3d inertia_tensor_reverse, affine_state;

	// Lame parameters
	double lambda, mu;

	// Deformation gradient (elastic and plastic parts)
	Eigen::Matrix3d def_elastic, def_plastic;

	// Cached SVD's for elastic deformation gradient
	Eigen::Matrix3d svd_w, svd_v;
	Eigen::Vector3d svd_e;

	// Grid interpolation weights
	Eigen::Vector3d grid_position;
	Eigen::Vector3d weight_gradient[64];
	double weights[64];

	Particle();
	Particle(const Eigen::Vector3d& pos, const Eigen::Vector3d& vel, double mass, double lambda, double mu);
	virtual ~Particle();

	// Update position, based on velocity
	void updatePos();

	// Update deformation gradient
	void updateGradient();
	void applyPlasticity();

	// Compute stress tensor
	const Eigen::Matrix3d energyDerivative();
};

#endif // !PARTICLE_H
