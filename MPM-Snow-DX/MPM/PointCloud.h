#pragma once
#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <vector>

#include "SimulationParameters.h"
#include "Particle.h"
#include "Entity.h"
#include "..\Content\ShaderStructures.h"


#define VOLUME_EPSILON 1e-5

inline double random_number(double low, double high)
{
	return low + rand() / (double)(RAND_MAX / (high - low));
}

class PointCloud
{
public:
	int size;
	double max_velocity;
	std::vector<Particle> particles;

	PointCloud();
	PointCloud(int cloud_size);
	PointCloud(const PointCloud& orig);
	virtual ~PointCloud();

	// Update particle data
	void update();

	// Get bounding box [vertex a, vertex b]
	void bounds(Eigen::Vector3d points[2]);

	// Generate particles that fill a set of shapes
	static PointCloud* createEntity(std::vector<Entity*>& snow_entities) {

		// Compute area of all the snow entities
		double volume = 0;
		int len = snow_entities.size(), num_shapes = 0;

		for (int i = 0; i < len; i++)
		{
			double indi_volume = snow_entities[i]->volume();
			if (indi_volume > VOLUME_EPSILON)
			{
				num_shapes++;
				volume += indi_volume;
			}
		}

		// A snow simulation cannot be done if there is no volume
		if (volume < VOLUME_EPSILON)
			return NULL;

		// Otherwise, create the object
		// Calculate particle settings
		double particle_volume = PARTICLE_DIAM * PARTICLE_DIAM * PARTICLE_DIAM,
			   particle_mass = particle_volume * DENSITY;

		int particles = volume / particle_volume;

		// Randomly scatter points
		PointCloud *obj = new PointCloud(particles);

		Eigen::Vector3d bounds[2];
		int shape_num = 0, total_points = 0;
		double max_vel = 0;
		for (int i = 0; i < len; i++)
		{
			double v = snow_entities[i]->volume();
			Eigen::Vector3d vel = snow_entities[i]->vel;

			if (lengthSquared(vel) > max_vel)
			{
				max_vel = lengthSquared(vel);
			}

			if (v > VOLUME_EPSILON)
			{
				int points;
				// Points given to each shape is proportional to their area
				if (++shape_num < num_shapes)
					points = v * particles / volume;
				// Last shape gets remainder, so we don't have round-off errors
				else points = particles - total_points;

				total_points += points;

				// Estimate the centroid of the shape with the bounds
				snow_entities[i]->bounds(bounds);
				double cx = (bounds[0](0) + bounds[1](0)) / 2.0,
					   cy = (bounds[0](1) + bounds[1](1)) / 2.0,
					   cz = (bounds[0](2) + bounds[1](2)) / 2.0,
					   cw = bounds[1](0) - cx,
					   ch = bounds[1](1) - cy,
				       cs = bounds[1](2) - cz;

				// Randomly scatter points in the shape until the quota is met
				int points_found = 0;
				srand(7);

				while (points_found != points)
				{
					double tx = random_number(bounds[0](0), bounds[1](0)),
					 	   ty = random_number(bounds[0](1), bounds[1](1)),
						   tz = random_number(bounds[0](2), bounds[1](2));

					// Check if this point is inside the shape
					if (snow_entities[i]->contains(tx, ty, tz))
					{
						// Randomly adjust hardness of snow
						double adjust = (rand() / (double)RAND_MAX) * 10;

						// Make snow hardest on the outer edges
						adjust *= ((fabs(tx - cx) / cw) + (fabs(ty - cy) / ch) + (fabs(tz - cz) / cs)) / 2.0;

						// Add the snow particle
						obj->particles.push_back(Particle(Eigen::Vector3d(tx, ty, tz), Eigen::Vector3d(vel), particle_mass, LAMBDA, MU));

						points_found++;
					}
				}
			}
		}

		// Set initial max velocity
		obj->max_velocity = max_vel;

		return obj;
	}

};

#endif