#include "pch.h"
#include "Scene.h"

Scene::Scene() {}
Scene::Scene(const Scene& scene) {}
Scene::~Scene() {}

Scene* Scene::GenerateScene(int index)
{
	Scene* scene = new Scene();

	switch (index)
	{
	case 0: {
		// Snowball crash
		Entity* snowball =
			Entity::generateSnowball(Eigen::Vector3d(1.05, 0.53, 0.5), 0.04, Eigen::Vector3d(-50, 0, 0));
		scene->snow_entities.push_back(snowball);

		Entity* snowball_2 =
			Entity::generateSnowball(Eigen::Vector3d(0.95, 0.47, 0.5), 0.04, Eigen::Vector3d(50, 0, 0));
		scene->snow_entities.push_back(snowball_2);
		break;
	}
	case 1: {
		// Snowball fall
		Entity * snowball =
			Entity::generateSnowball(Eigen::Vector3d(1, 0.5, 0.5), 0.1, Eigen::Vector3d(0, -10, 0));
		scene->snow_entities.push_back(snowball);
		break;
	}
	case 2: {
		// Snowball heap
		Entity * snowball =
			Entity::generateSnowball(Eigen::Vector3d(1, 0.475, 0.5), 0.05, Eigen::Vector3d(0, 0, 0));
		scene->snow_entities.push_back(snowball);

		Entity* snowball_2 =
			Entity::generateSnowball(Eigen::Vector3d(1, 0.55, 0.5), 0.04, Eigen::Vector3d(0, 0, 0));
		scene->snow_entities.push_back(snowball_2);

		Entity* snowball_3 =
			Entity::generateSnowball(Eigen::Vector3d(1.15, 0.55, 0.5), 0.03, Eigen::Vector3d(-90, 0, 0));
		scene->snow_entities.push_back(snowball_3);
		break;
	}
	case 3: {
		// Static snowball smash
		Entity* snowball =
			Entity::generateSnowball(Eigen::Vector3d(1.1, 0.5, 0.5), 0.03, Eigen::Vector3d(-75, 0, 0));
		scene->snow_entities.push_back(snowball);

		Entity* snowball_2 =
			Entity::generateSnowball(Eigen::Vector3d(0.93, 0.5, 0.5), 0.07, Eigen::Vector3d(0, 0, 0));
		scene->snow_entities.push_back(snowball_2);
		break;
	}
	case 4: {
		// Snowball smash
		Entity* snowball =
			Entity::generateSnowball(Eigen::Vector3d(1.1, 0.5, 0.5), 0.04, Eigen::Vector3d(-45, 0, 0));
		scene->snow_entities.push_back(snowball);

		Entity* snowball_2 =
			Entity::generateSnowball(Eigen::Vector3d(0.9, 0.5, 0.5), 0.04, Eigen::Vector3d(45, 0, 0));
		scene->snow_entities.push_back(snowball_2);
		break;
	}
	case 5: {
		// Small snowcube and snowball
		Entity* snowball =
			Entity::generateSnowball(Eigen::Vector3d(1.1, 0.5, 0.5), 0.03, Eigen::Vector3d(-75, 0, 0));
		scene->snow_entities.push_back(snowball);

		Entity* snowcube =
			Entity::generateSnowcube(Eigen::Vector3d(0.9, 0.5, 0.5), 0.07, Eigen::Vector3d(0, 0, 0));
		scene->snow_entities.push_back(snowcube);
		break;
	}
	case 6: {
		// Large snowcube and snowball
		Entity* snowball =
			Entity::generateSnowball(Eigen::Vector3d(1.1, 0.5, 0.5), 0.03, Eigen::Vector3d(-75, 0, 0));
		scene->snow_entities.push_back(snowball);

		Entity* snowcube =
			Entity::generateSnowcube(Eigen::Vector3d(0.9, 0.5, 0.5), 0.10, Eigen::Vector3d(0, 0, 0));
		scene->snow_entities.push_back(snowcube);
		break;
	}
	case 7: {
		// Snowall and snowball
		Entity* snowball =
			Entity::generateSnowball(Eigen::Vector3d(1.1, 0.5, 0.5), 0.03, Eigen::Vector3d(-75, 0, 0));
		scene->snow_entities.push_back(snowball);

		Entity* snowcube =
			Entity::generateSnowcube(Eigen::Vector3d(0.9, 0.5, 0.5), Eigen::Vector3d(0.03, 0.2, 0.05), Eigen::Vector3d(0, 0, 0));
		scene->snow_entities.push_back(snowcube);
		break;
	}
	case 8: {
		// Snowcube smash
		Entity* snowcube_1 =
			Entity::generateSnowcube(Eigen::Vector3d(1.1, 0.5, 0.5), 0.05, Eigen::Vector3d(-40, 0, 0));
		scene->snow_entities.push_back(snowcube_1);

		Entity* snowcube_2 =
			Entity::generateSnowcube(Eigen::Vector3d(0.9, 0.5, 0.5), 0.05, Eigen::Vector3d(40, 0, 0));
		scene->snow_entities.push_back(snowcube_2);
		break;
	}
	case 9: {
		// Snowcube crash
		Entity* snowcube_1 =
			Entity::generateSnowcube(Eigen::Vector3d(1.1, 0.58, 0.5), Eigen::Vector3d(0.03, 0.2, 0.05), Eigen::Vector3d(-40, 0, 0));
		scene->snow_entities.push_back(snowcube_1);

		Entity* snowcube_2 =
			Entity::generateSnowcube(Eigen::Vector3d(0.9, 0.42, 0.5), Eigen::Vector3d(0.03, 0.2, 0.05), Eigen::Vector3d(40, 0, 0));
		scene->snow_entities.push_back(snowcube_2);
		break;
	}
	case 10: {
		// Snowcube (hori) and snowball
		Entity* snowball =
			Entity::generateSnowball(Eigen::Vector3d(1.1, 0.5, 0.5), 0.03, Eigen::Vector3d(-75, 0, 0));
		scene->snow_entities.push_back(snowball);

		Entity* snowcube =
			Entity::generateSnowcube(Eigen::Vector3d(0.85, 0.5, 0.5), Eigen::Vector3d(0.2, 0.04, 0.04), Eigen::Vector3d(0, 0, 0));
		scene->snow_entities.push_back(snowcube);
		break;
	}
	case 11: {
		// Snowstud
		Entity* snowcube_1 =
			Entity::generateSnowcube(Eigen::Vector3d(1.15, 0.5, 0.5), Eigen::Vector3d(0.2, 0.04, 0.04), Eigen::Vector3d(-75, 0, 0));
		scene->snow_entities.push_back(snowcube_1);

		Entity* snowcube_2 =
			Entity::generateSnowcube(Eigen::Vector3d(0.8, 0.5, 0.5), 0.15, Eigen::Vector3d(0, 0, 0));
		scene->snow_entities.push_back(snowcube_2);
		break;
	}
	default: {
		//std::cout << "\nScene index out of range." << std::endl;
		break;
	}
	}

	return scene;
}