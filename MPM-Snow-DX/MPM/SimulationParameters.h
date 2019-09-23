#pragma once
#ifndef SIMPARAMETERS_H
#define SIMPARAMETERS_H

#include <Eigen\Dense>

static const double
PARTICLE_DIAM = 0.005,		// Diameter of each particle; smaller = higher resolution
MAX_TIMESTEP = 5e-4,		// Upper timestep limit
CRIT_COMPRESS = 1 - 2.5e-2,	// Fracture threshold for compression (1-2.5e-2)
CRIT_STRETCH = 1 + 7.5e-3,	// Fracture threshold for stretching (1+7.5e-3)
HARDENING = 10.0,			// How much plastic deformation strengthens material (10)
DENSITY = 400,				// Density of snow in kg/m^3
YOUNGS_MODULUS = 1.4e5,		// Young's modulus (springiness) (1.4e5)
POISSONS_RATIO = 0.2,		// Poisson's ratio (transverse/axial strain ratio) (.2)
STICKY = 0.5,				// Collision stickiness (lower = stickier)
TIMESTEP = 0.00005;			// Simulation timestep

// Hardening parameters
static const double
LAMBDA = YOUNGS_MODULUS * POISSONS_RATIO / ((1 + POISSONS_RATIO)*(1 - 2 * POISSONS_RATIO)),
MU = YOUNGS_MODULUS / (2 + 2 * POISSONS_RATIO);

static const Eigen::Vector3d GRAVITY = Eigen::Vector3d(0, -9.8, 0);

// Dimension properties
#define WIN_METERS_X 2
#define WIN_METERS_Y 1
#define WIN_METERS_Z 1

#define GRID_RES_X 256
#define GRID_RES_Y 128
#define GRID_RES_Z 128

#endif // !SIMPARAMETERS_H
