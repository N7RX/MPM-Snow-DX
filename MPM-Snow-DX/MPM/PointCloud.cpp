#include "pch.h"
#include "PointCloud.h"

PointCloud::PointCloud() {}

PointCloud::PointCloud(int cloud_size)
{
	size = cloud_size;
	particles.reserve(size);
}

// Copy constructor
PointCloud::PointCloud(const PointCloud& orig) {}
PointCloud::~PointCloud() {}


// Update particle data
void PointCloud::update()
{
	max_velocity = 0;

	for (int i = 0; i < size; i++)
	{
		particles[i].updatePos();
		particles[i].updateGradient();
		particles[i].applyPlasticity();

		// Update max velocity, if needed
		double vel = lengthSquared(particles[i].velocity);
		if (vel > max_velocity)
		{
			max_velocity = vel;
		}
	}
}


// Get bounding box [vertex a, vertex b]
void PointCloud::bounds(Eigen::Vector3d points[2])
{
	points[0](0) = particles[0].position(0); points[1](0) = points[0](0);
	points[0](1) = particles[0].position(1); points[1](1) = points[0](1);
	points[0](2) = particles[0].position(2); points[1](2) = points[0](2);

	for (int i = 0; i<size; i++)
	{
		Eigen::Vector3d& p = particles[i].position;
		// X-bounds
		if (p(0) < points[0](0))
			points[0](0) = p(0);
		else if (p(0) > points[1](0))
			points[1](0) = p(0);

		// Y-bounds
		if (p(1) < points[0](1))
			points[0](1) = p(1);
		else if (p(1) > points[1](1))
			points[1](1) = p(1);

		// Z-bounds
		if (p(2) < points[0](2))
			points[0](2) = p(2);
		else if (p(2) > points[1](2))
			points[1](2) = p(2);
	}
}