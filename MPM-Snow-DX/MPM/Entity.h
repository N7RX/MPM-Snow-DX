#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <math.h>

#include <Eigen\Dense>
#include "CustomMath.h"

const double Pi4_3 = 3.1415926535 * 4 / 3;

// A simplified 3D version, supports only sphere and cube
class Entity
{
public:
	enum ShapeType
	{
		Sphere,
		Cube
	};
	ShapeType type;
	
	Eigen::Vector3d center;
	// Sphere
	double radius;
	// Cube
	double edge_length[3];

	Eigen::Vector3d vel;

	Entity();
	Entity(Eigen::Vector3d vel);
	Entity(const Entity& orig);
	virtual ~Entity();

	// Does this shape contain this point
	bool contains(double x, double y, double z);

	// Compute volume
	float volume();

	// Bounding box for shape
	void bounds(Eigen::Vector3d points[2]);

	static Entity* generateSnowball(Eigen::Vector3d origin, double radius, Eigen::Vector3d vel);
	static Entity* generateSnowcube(Eigen::Vector3d origin, Eigen::Vector3d edgeLength, Eigen::Vector3d vel);
	static Entity* generateSnowcube(Eigen::Vector3d origin, double squareLength, Eigen::Vector3d vel);

};

#endif // !ENTITY_H
