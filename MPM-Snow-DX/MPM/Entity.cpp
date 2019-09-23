#include "pch.h"
#include "Entity.h"

Entity::Entity() {}
Entity::Entity(Eigen::Vector3d vel) :vel(vel) {}
// Copy constructor
Entity::Entity(const Entity& orig) {}

Entity::~Entity() {}


bool Entity::contains(double x, double y, double z)
{
	if (type == ShapeType::Sphere)
	{
		Eigen::Vector3d line = Eigen::Vector3d(x, y, z) - center;
		return lengthSquared(line) <= radius * radius;
	}
	if (type == ShapeType::Cube)
	{
		if (fabs(x - center(0)) > edge_length[0] / 2
			|| fabs(y - center(1)) > edge_length[1] / 2
			|| fabs(z - center(2)) > edge_length[2] / 2)
		{
			return false;
		}
	}
	return true;
}


float Entity::volume()
{
	if (type == ShapeType::Sphere)
	{
		return Pi4_3 * radius*radius*radius;
	}
	if (type == ShapeType::Cube)
	{
		return edge_length[0] * edge_length[1] * edge_length[2];
	}
	return 0;
}


void Entity::bounds(Eigen::Vector3d points[2])
{
	if (type == ShapeType::Sphere)
	{
		points[0] = center + Eigen::Vector3d(radius, radius, radius);
		points[1] = center - Eigen::Vector3d(radius, radius, radius);
	}
	if (type == ShapeType::Cube)
	{
		points[0] = center + Eigen::Vector3d(edge_length[0], edge_length[1], edge_length[2]);
		points[1] = center - Eigen::Vector3d(edge_length[0], edge_length[1], edge_length[2]);
	}
}


// Generate a specific snow ball
Entity* Entity::generateSnowball(Eigen::Vector3d origin, double radius, Eigen::Vector3d vel)
{
	Entity* snowball = new Entity(vel);

	snowball->center = origin;
	snowball->radius = radius;
	snowball->type = ShapeType::Sphere;

	return snowball;
}


// Generate a specific snow cube
Entity* Entity::generateSnowcube(Eigen::Vector3d origin, Eigen::Vector3d edgeLength, Eigen::Vector3d vel)
{
	Entity* snowcube = new Entity(vel);

	snowcube->center = origin;
	snowcube->edge_length[0] = edgeLength(0);
	snowcube->edge_length[1] = edgeLength(1);
	snowcube->edge_length[2] = edgeLength(2);
	snowcube->type = ShapeType::Cube;

	return snowcube;
}

Entity* Entity::generateSnowcube(Eigen::Vector3d origin, double squareLength, Eigen::Vector3d vel)
{
	Eigen::Vector3d l = Eigen::Vector3d(squareLength,squareLength ,squareLength);
	return generateSnowcube(origin, l, vel);
}