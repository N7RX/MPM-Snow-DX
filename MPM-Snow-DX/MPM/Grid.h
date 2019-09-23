#pragma once
#ifndef GRID_H
#define GRID_H

#include <math.h>
#include <cstring>
#include <stdio.h>
#include <iostream>

#include "SimulationParameters.h"
#include "PointCloud.h"

const double BSPLINE_EPSILON = 1e-4;
const int   BSPLINE_RADIUS = 2;

// Grid node data
struct GridNode
{
	double mass;
	bool active;
	Eigen::Vector3d velocity, velocity_new;
};

class Grid
{
public:
	Eigen::Vector3d origin, size, cellsize;
	PointCloud* point_cloud;
	double node_volume;
	// Nodes: use (y*size[0]*size[2] + z*size[0] + x) to index, where zero is the bottom-left corner (e.g. like a cartesian grid)
	int nodes_length;
	GridNode* nodes;

	// APIC: grid node position
	// Stored outside GridNode to prevent this data being cleared
	Eigen::Vector3d* nodes_position;

	// Grid should be at least one cell; there must be one layer of cells surrounding all particles
	Grid(Eigen::Vector3d pos, Eigen::Vector3d dims, Eigen::Vector3d cells, PointCloud* obj);
	Grid(const Grid& orig);
	virtual ~Grid();

	// Map particles to grid
	void initializeMass();
	void initializeVelocities();

	// APIC: initialize inertia-like tensor matrix
	void initializeInertiaTensor();

	// Map grid volumes back to particles (first timestep only)
	void calculateVolumes() const;

	// Compute grid velocities
	void explicitVelocities(const Eigen::Vector3d& gravity);

	// APIC: update affine matrix component of particles
	void updateAffineState() const;

	// Map grid velocities back to particles
	void updateVelocities() const;

	// Collision detection
	void collisionGrid();
	void collisionParticles() const;

	// One-dimensional cubic B-splines
	// A smooth curve from (0,1) to (1,0)
	static double B_Spline(double x)
	{
		x = fabs(x);
		double w;

		if (x < 1)
		{
			w = x * x * (x / 2 - 1) + 2 / 3.0;
		}
		else if (x < 2)
		{
			w = x * (x * (-x / 6 + 1) - 2) + 4 / 3.0;
		}
		else return 0;
		
		if (w < BSPLINE_EPSILON)
		{
			return 0;
		}

		return w;
	}
	// Slope of interpolation function (derivative)
	static double B_SplineSlope(double x)
	{
		double abs_x = fabs(x);

		if (abs_x < 1)
		{
			return 1.5 * x * abs_x - 2 * x;
		}
		else if (x < 2)
		{
			return -x * abs_x / 2 + 2 * x - 2 * x / abs_x;
		}
		else return 0;
	}

};

#endif // !GRID_H
