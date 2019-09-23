#pragma once
#ifndef SCENE_H
#define SCENE_H

#include <stdlib.h>
#include "Entity.h"

class Scene
{
public:
	std::vector<Entity*> snow_entities;

	Scene();
	Scene(const Scene&);
	virtual ~Scene();

	static Scene* GenerateScene(int); // 0-Snow ball smash;

};

#endif // !SCENE_H
