#pragma once
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdlib.h>

#include "PointCloud.h"
#include "Grid.h"
#include "Particle.h"
#include "PointCloud.h"
#include "Grid.h"
#include "SimulationParameters.h"
#include "Entity.h"
#include "Scene.h"

class Simulator
{
public:
	Grid* grid;
	PointCloud* point_cloud;

	Simulator(Scene* scene);
	Simulator(const Simulator& orig);
	virtual ~Simulator();

	void update();
};

#endif // !SIMULATOR_H
