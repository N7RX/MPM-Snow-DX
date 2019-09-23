#include "pch.h"
#include "Particle.h"

Particle::Particle() {}

Particle::Particle(const Eigen::Vector3d& pos, const Eigen::Vector3d& vel, double mass, double lame_lambda, double lame_mu)
{
	position = pos;
	velocity = vel;
	this->mass = mass;
	lambda = lame_lambda;
	mu = lame_mu;
	// To start out with, assume the deformation gradient is zero
	// Or in other words, all particle velocities are the same
	loadIdentity(def_elastic);
	loadIdentity(def_plastic);
	setData(svd_e, 1, 1, 1);
	loadIdentity(svd_w);
	loadIdentity(svd_v);
}

Particle::~Particle() {}

// Update position, based on velocity
void Particle::updatePos() 
{
	// Simple euler integration
	position += TIMESTEP * velocity;
}

// Update deformation gradient
void Particle::updateGradient()
{
	// Initially make all updates elastic
	velocity_gradient *= TIMESTEP;
	diagSum(velocity_gradient, 1);
	def_elastic = velocity_gradient * def_elastic;
}

void Particle::applyPlasticity()
{
	// Compute the SVD decomposition
	// The singular values (basically a scale transform) tell us if 
	// the particle has exceeded critical stretch/compression
	Eigen::JacobiSVD<Eigen::Matrix3d, Eigen::NoQRPreconditioner> svd;

	// Compute singular value decomposition (uev*)
	svd.compute(def_elastic, Eigen::ComputeFullV | Eigen::ComputeFullU);

	svd_w = svd.matrixU();
	svd_v = svd.matrixV();
	svd_e = svd.singularValues();

	// Clamp singular values to within elastic region
	for (int i = 0; i < 3; i++)
	{
		if (svd_e[i] < CRIT_COMPRESS)
			svd_e[i] = CRIT_COMPRESS;

		else if (svd_e[i] > CRIT_STRETCH)
			svd_e[i] = CRIT_STRETCH;
	}

	// Recompute elastic and plastic gradient
	// Basically just putting the SVD back together again
	def_plastic = svd_v * svd_e.asDiagonal().inverse() * svd_w.transpose() * def_elastic * def_plastic;
	svd_v.transposeInPlace();
	def_elastic = svd_w * svd_e.asDiagonal() * svd_v;
}

// Compute stress tensor
const Eigen::Matrix3d Particle::energyDerivative()
{
	Eigen::Matrix3d energy = 2 * mu*(def_elastic - svd_w * svd_v)*def_elastic.transpose();
	//Je is the determinant of def_elastic (equivalent to svd_e.prod())
	double Je = svd_e.prod(),
		   contour = lambda * Je*(Je - 1),
		   Jp = def_plastic.determinant();

	for (int i = 0; i < 3; i++)
	{
		energy(i, i) += contour;
	}

	energy *= volume * exp(HARDENING*(1 - Jp));

	return energy;
}
